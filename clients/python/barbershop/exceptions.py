"Core exceptions raised by the Barbershop client"

class BarbershopError(Exception):
    pass
    
class AuthenticationError(BarbershopError):
    pass
    
class ConnectionError(BarbershopError):
    pass
    
class ResponseError(BarbershopError):
    pass
    
class InvalidResponse(BarbershopError):
    pass
    
class InvalidData(BarbershopError):
    pass
    
