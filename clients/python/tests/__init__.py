import unittest
from server_commands import ServerCommandsTestCase
from connection_pool import ConnectionPoolTestCase

def all_tests():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(ServerCommandsTestCase))
    return suite
