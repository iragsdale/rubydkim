#include "ruby.h"
#include "pdkim1.h"

#ifdef HAVE_RUBY_IO_H
  #include "ruby/io.h"
  #define GetWriteFile(fp) rb_io_stdio_file(fp)
  #define OpenFile rb_io_t
#else
  #include "rubyio.h"
#endif

#ifndef STR2CSTR
#define STR2CSTR StringValueCStr
#endif

static VALUE mDKIM, cDKIMSigner, cDKIMSignature, cTime;

// feed data to the dkim context
VALUE signer_feed(VALUE obj, VALUE text)
{                 
    // get the dkim context
    pdkim_ctx *ctx;
    Data_Get_Struct(obj, pdkim_ctx, ctx);
    
    // get the text data
    char * data = StringValuePtr(text);
    
    // pass the string data to pdkim_feed while setting the length
    if ( pdkim_feed(ctx, data, strlen(data)) != PDKIM_OK) {
        printf("pdkim_feed() error\n");
    }
    return Qnil;
}

// create a signature object from a pdkim_signature
VALUE new_signature(pdkim_signature *sig)
{
    VALUE rsig = rb_funcall(cDKIMSignature, rb_intern("new"), 0);
    if (sig->version > 0)
        rb_iv_set(rsig, "@version", INT2FIX(sig->version));
    rb_iv_set(rsig, "@algo", INT2FIX(sig->algo));
    rb_iv_set(rsig, "@canon_headers", INT2FIX(sig->canon_headers));
    rb_iv_set(rsig, "@canon_body", INT2FIX(sig->canon_body));
    rb_iv_set(rsig, "@querymethod", INT2FIX(sig->querymethod));
    if (sig->selector != NULL)
        rb_iv_set(rsig, "@selector", rb_str_new2(sig->selector));
    if (sig->domain != NULL)
        rb_iv_set(rsig, "@domain", rb_str_new2(sig->domain));
    if (sig->identity != NULL)
        rb_iv_set(rsig, "@identity", rb_str_new2(sig->identity));
    if (sig->created > 0)
        rb_iv_set(rsig, "@created", rb_funcall(cTime, rb_intern("at"), 1, INT2FIX(sig->created)));
    if (sig->expires > 0)
        rb_iv_set(rsig, "@expires", rb_funcall(cTime, rb_intern("at"), 1, INT2FIX(sig->expires)));
    rb_iv_set(rsig, "@bodylength", INT2FIX(sig->bodylength));
    if (sig->headernames != NULL)
        rb_iv_set(rsig, "@headernames", rb_str_new2(sig->headernames));
    if (sig->copiedheaders != NULL)
        rb_iv_set(rsig, "@copiedheaders", rb_str_new2(sig->copiedheaders));
    if (sig->signature_header != NULL)
        rb_iv_set(rsig, "@signature_header", rb_str_new2(sig->signature_header));
    rb_iv_set(rsig, "@verify_status", INT2FIX(sig->verify_status));
    rb_iv_set(rsig, "@verify_ext_status", INT2FIX(sig->verify_ext_status));
    // rb_iv_set(rsig, "@pubkey", );
    return rsig;
}

// finish sending message data and return the signature
VALUE signer_finish(VALUE obj)
{
    // get the dkim context
    pdkim_ctx *ctx;
    Data_Get_Struct(obj, pdkim_ctx, ctx);
    
    // create a signature object
    pdkim_signature *signature;
    int result = pdkim_feed_finish(ctx,&signature);
    if ( result == PDKIM_OK ) {
        return new_signature(signature);   
    }
    else {
        rb_raise(rb_eRuntimeError, "error finishing signature: %d", result);
    }
}

// frees the class when we're done
static void signer_free(void *ctx) {
  pdkim_free_ctx(ctx);
}

// initializes the signer object
static VALUE signer_init(VALUE self) {
  return self;
}

// allocates a new dkim object
VALUE signer_new(VALUE class, VALUE domain, VALUE selector, VALUE key)
{
    // create the DKIM context
    pdkim_ctx *ctx = pdkim_init_sign(PDKIM_INPUT_NORMAL,          /* Input type */
                                       StringValuePtr(domain),    /* Domain   */
                                       StringValuePtr(selector),  /* Selector */
                                       StringValuePtr(key)        /* Private RSA key */
                       );
    
    // wrap it into our class
    VALUE obj = Data_Wrap_Struct(class, 0, signer_free, ctx);
    
    // initialize and return it
    rb_obj_call_init(obj, 0, 0);
    return obj;
}

// sends debugging output to the given file
VALUE signer_debug(VALUE obj, VALUE file)
{   
    // get the dkim context
    pdkim_ctx *ctx;
    Data_Get_Struct(obj, pdkim_ctx, ctx);
    
    // if we were given a nil object, turn debugging off
    if (TYPE(file) == T_NIL) {
        pdkim_set_debug_stream(ctx, NULL);
    }
    // if we were given a file, use it for debugging
    else if (TYPE(file) == T_FILE) {
        OpenFile *openfile = NULL;
        GetOpenFile(file, openfile);
        pdkim_set_debug_stream(ctx, GetWriteFile(openfile));
    }
    // otherwise, raise an exception
    else {
        rb_raise(rb_eTypeError, "debug requires a file handle");
    }
    return T_NONE;
}

// defines the new ruby class and hooks up the proper methods
void Init_signer() {
  mDKIM = rb_define_module("DKIM");
  cDKIMSigner = rb_define_class_under(mDKIM, "Signer", rb_cObject);
  cDKIMSignature = rb_define_class_under(mDKIM, "Signature", rb_cObject);
  cTime = rb_define_class("Time", rb_cObject);
  rb_define_singleton_method(cDKIMSigner, "new", signer_new, 3);
  rb_define_method(cDKIMSigner, "initialize", signer_init, 0);
  rb_define_method(cDKIMSigner, "feed", signer_feed, 1);
  rb_define_method(cDKIMSigner, "finish", signer_finish, 0);
  rb_define_method(cDKIMSigner, "debug", signer_debug, 1);
}

