package CORBAServer;

/**
* CORBAServer/CorbaInetAddressExceptionHolder.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from /home/starcraftman/programming/423DistributedProject/a2/423-a2/src/corba/base/interface.idl
* Sunday, November 4, 2012 6:35:54 o'clock AM EST
*/

public final class CorbaInetAddressExceptionHolder implements org.omg.CORBA.portable.Streamable
{
  public CORBAServer.CorbaInetAddressException value = null;

  public CorbaInetAddressExceptionHolder ()
  {
  }

  public CorbaInetAddressExceptionHolder (CORBAServer.CorbaInetAddressException initialValue)
  {
    value = initialValue;
  }

  public void _read (org.omg.CORBA.portable.InputStream i)
  {
    value = CORBAServer.CorbaInetAddressExceptionHelper.read (i);
  }

  public void _write (org.omg.CORBA.portable.OutputStream o)
  {
    CORBAServer.CorbaInetAddressExceptionHelper.write (o, value);
  }

  public org.omg.CORBA.TypeCode _type ()
  {
    return CORBAServer.CorbaInetAddressExceptionHelper.type ();
  }

}
