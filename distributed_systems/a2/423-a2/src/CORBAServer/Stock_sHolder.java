package CORBAServer;

/**
* CORBAServer/Stock_sHolder.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from /home/starcraftman/programming/423DistributedProject/a2/423-a2/src/corba/base/interface.idl
* Sunday, November 4, 2012 6:35:54 o'clock AM EST
*/

public final class Stock_sHolder implements org.omg.CORBA.portable.Streamable
{
  public CORBAServer.Stock_s value = null;

  public Stock_sHolder ()
  {
  }

  public Stock_sHolder (CORBAServer.Stock_s initialValue)
  {
    value = initialValue;
  }

  public void _read (org.omg.CORBA.portable.InputStream i)
  {
    value = CORBAServer.Stock_sHelper.read (i);
  }

  public void _write (org.omg.CORBA.portable.OutputStream o)
  {
    CORBAServer.Stock_sHelper.write (o, value);
  }

  public org.omg.CORBA.TypeCode _type ()
  {
    return CORBAServer.Stock_sHelper.type ();
  }

}
