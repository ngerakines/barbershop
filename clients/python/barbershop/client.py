import datetime
import errno
import socket
import threading
import warnings
from barbershop.exceptions import ConnectionError, ResponseError, InvalidResponse
from barbershop.exceptions import BarbershopError, AuthenticationError

class ConnectionManager(threading.local):
    "Manages a list of connections on the local thread"
    def __init__(self):
        self.connections = {}
        
    def make_connection_key(self, host, port):
        "Create a unique key for the specified host and port"
        return '%s:%s' % (host, port)
        
    def get_connection(self, host, port):
        "Return a specific connection for the specified host, port"
        key = self.make_connection_key(host, port)
        if key not in self.connections:
            self.connections[key] = Connection(host, port)
        return self.connections[key]
        
    def get_all_connections(self):
        "Return a list of all connection objects the manager knows about"
        return self.connections.values()
        
connection_manager = ConnectionManager()

class Connection(object):
    "Manages TCP communication to and from a Barbershop server"
    def __init__(self, host='localhost', port=6379):
        self.host = host
        self.port = port
        self._sock = None
        self._fp = None
        
    def connect(self, bs_instance):
        "Connects to the Barbershop server is not already connected"
        if self._sock:
            return
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self.host, self.port))
        except socket.error, e:
            raise ConnectionError("Error %s connecting to %s:%s. %s." % \
                (e.args[0], self.host, self.port, e.args[1]))
        sock.setsockopt(socket.SOL_TCP, socket.TCP_NODELAY, 1)
        self._sock = sock
        self._fp = sock.makefile('r')
        
    def disconnect(self):
        "Disconnects from the Barbershop server"
        if self._sock is None:
            return
        try:
            self._sock.close()
        except socket.error:
            pass
        self._sock = None
        self._fp = None
        
    def send(self, command, barbershop_instance):
        "Send ``command`` to the Barbershop server. Return the result."
        self.connect(barbershop_instance)
        try:
            self._sock.sendall(command)
        except socket.error, e:
            if e.args[0] == errno.EPIPE:
                self.disconnect()
            raise ConnectionError("Error %s while writing to socket. %s." % \
                e.args)
                
    def read(self, length=None):
        """
        Read a line from the socket is length is None,
        otherwise read ``length`` bytes
        """
        try:
            if length is not None:
                return self._fp.read(length)
            return self._fp.readline()
        except socket.error, e:
            self.disconnect()
            if e.args and e.args[0] == errno.EAGAIN:
                raise ConnectionError("Error while reading from socket: %s" % \
                    e.args[1])
        return ''
        
def list_or_args(command, keys, args):
    # returns a single list combining keys and args
    # if keys is not a list or args has items, issue a
    # deprecation warning
    oldapi = bool(args)
    try:
        i = iter(keys)
        # a string can be iterated, but indicates
        # keys wasn't passed as a list
        if isinstance(keys, basestring):
            oldapi = True
    except TypeError:
        oldapi = True
        keys = [keys]
    if oldapi:
        warnings.warn(DeprecationWarning(
            "Passing *args to Barbershop.%s has been deprecated. "
            "Pass an iterable to ``keys`` instead" % command
        ))
        keys.extend(args)
    return keys
    
def timestamp_to_datetime(response):
    "Converts a unix timestamp to a Python datetime object"
    if not response:
        return None
    try:
        response = int(response)
    except ValueError:
        return None
    return datetime.datetime.fromtimestamp(response)
    
def string_keys_to_dict(key_string, callback):
    return dict([(key, callback) for key in key_string.split()])

def dict_merge(*dicts):
    merged = {}
    [merged.update(d) for d in dicts]
    return merged
    
def parse_info(response):
    "Parse the result of Barbershop's INFO command into a Python dict"
    info = {}
    def get_value(value):
        if ',' not in value:
            return value
        sub_dict = {}
        for item in value.split(','):
            k, v = item.split('=')
            try:
                sub_dict[k] = int(v)
            except ValueError:
                sub_dict[k] = v
        return sub_dict
    for line in response.splitlines():
        key, value = line.split(':')
        try:
            info[key] = int(value)
        except ValueError:
            info[key] = get_value(value)
    return info
    
def zset_score_pairs(response, **options):
    """
    If ``withscores`` is specified in the options, return the response as
    a list of (value, score) pairs
    """
    if not response or not options['withscores']:
        return response
    return zip(response[::2], map(float, response[1::2]))
    
    
class Barbershop(threading.local):
    """
    Implementation of the Barbershop protocol.
    
    This abstract class provides a Python interface to all Barbershop commands
    and an implementation of the Barbershop protocol.
    
    Connection and Pipeline derive from this, implementing how
    the commands are sent and received to the Barbershop server
    """
    RESPONSE_CALLBACKS = dict_merge(
        string_keys_to_dict(
            'AUTH DEL EXISTS EXPIRE HDEL MOVE MSETNX RENAMENX '
            'SADD SISMEMBER SMOVE SETNX SREM ZADD ZREM',
            bool
            ),
        string_keys_to_dict(
            'DECRBY INCRBY LLEN SCARD SDIFFSTORE SINTERSTORE SUNIONSTORE '
            'ZCARD ZRANK ZREMRANGEBYSCORE ZREVRANK',
            int
            ),
        string_keys_to_dict(
            # these return OK, or int if barbershop-server is >=1.3.4
            'LPUSH RPUSH',
            lambda r: isinstance(r, int) and r or r == 'OK'
            ),
        string_keys_to_dict('ZSCORE ZINCRBY',
            lambda r: r is not None and float(r) or r),
        string_keys_to_dict(
            'FLUSHALL FLUSHDB LSET LTRIM MSET RENAME '
            'SAVE SELECT SET SHUTDOWN',
            lambda r: r == 'OK'
            ),
        string_keys_to_dict('SDIFF SINTER SMEMBERS SUNION',
            lambda r: r and set(r) or r
            ),
        string_keys_to_dict('ZRANGE ZRANGEBYSCORE ZREVRANGE', zset_score_pairs),
        {
            'BGSAVE': lambda r: r == 'Background saving started',
            'INFO': parse_info,
            'LASTSAVE': timestamp_to_datetime,
            'PING': lambda r: r == 'PONG',
            'RANDOMKEY': lambda r: r and r or None,
            'TTL': lambda r: r != -1 and r or None,
        }
        )
    
    def __init__(self, host='localhost', port=8002, charset='utf-8', errors='strict'):
        self.encoding = charset
        self.errors = errors
        self.select(host, port)
 
    def select(self, host, port):
        """
        Switch to a different database on the current host/port
        
        Note this method actually replaces the underlying connection object
        prior to issuing the SELECT command.  This makes sure we protect
        the thread-safe connections
        """
        self.connection = self.get_connection(host, port)

    #### Legacty accessors of connection information ####
    def _get_host(self):
        return self.connection.host
    host = property(_get_host)
    
    def _get_port(self):
        return self.connection.port
    port = property(_get_port)
    
    #### COMMAND EXECUTION AND PROTOCOL PARSING ####
    def _execute_command(self, command_name, command, **options):
        self.connection.send(command, self)
        return self.parse_response(command_name, **options)

    def execute_command(self, command_name, command, **options):
        "Sends the command to the Barbershop server and returns it's response"
        try:
            return self._execute_command(command_name, command, **options)
        except ConnectionError:
            self.connection.disconnect()
            return self._execute_command(command_name, command, **options)
        
    def _parse_response(self, command_name, catch_errors):
        conn = self.connection
        response = conn.read().strip()
        if not response:
            self.connection.disconnect()
            raise ConnectionError("Socket closed on remote end")
            
        # server returned a null value
        if response in ('$-1', '*-1'):
            return None
        byte, response = response[0], response[1:]
        
        # server returned an error
        if byte == '-':
            if response.startswith('ERR '):
                response = response[4:]
            raise ResponseError(response)
        # single value
        elif byte == '+':
            return response
        # int value
        elif byte == ':':
            return int(response)
        # bulk response
        elif byte == '$':
            length = int(response)
            if length == -1:
                return None
            response = length and conn.read(length) or ''
            conn.read(2) # read the \r\n delimiter
            return response
        # multi-bulk response
        elif byte == '*':
            length = int(response)
            if length == -1:
                return None
            if not catch_errors:
                return [self._parse_response(command_name, catch_errors) 
                    for i in range(length)]
            else:
                # for pipelines, we need to read everything, including response errors.
                # otherwise we'd completely mess up the receive buffer
                data = []
                for i in range(length):
                    try:
                        data.append(
                            self._parse_response(command_name, catch_errors)
                            )
                    except Exception, e:
                        data.append(e)
                return data
            
        raise InvalidResponse("Unknown response type for: %s" % command_name)
        
    def parse_response(self, command_name, catch_errors=False, **options):
        "Parses a response from the Barbershop server"
        response = self._parse_response(command_name, catch_errors)
        if command_name in self.RESPONSE_CALLBACKS:
            return self.RESPONSE_CALLBACKS[command_name](response, **options)
        return response
        
    def encode(self, value):
        "Encode ``value`` using the instance's charset"
        if isinstance(value, str):
            return value
        if isinstance(value, unicode):
            return value.encode(self.encoding, self.errors)
        # not a string or unicode, attempt to convert to a string
        return str(value)
        
    def format_inline(self, *args, **options):
        "Formats a request with the inline protocol"
        cmd = '%s\r\n' % ' '.join([self.encode(a) for a in args])
        return self.execute_command(args[0], cmd, **options)
        
    #### CONNECTION HANDLING ####
    def get_connection(self, host, port):
        "Returns a connection object"
        conn = connection_manager.get_connection(host, port)
        return conn

    def info(self):
        "Returns a dictionary containing information about the Barbershop server"
        return self.format_inline('INFO')

    #### BASIC KEY COMMANDS ####
    def update(self, name, amount=1):
        return self.format_inline('UPDATE', name, amount)
    def next(self):
        return self.format_inline('NEXT')
    def peek(self):
        return self.format_inline('PEEK')

