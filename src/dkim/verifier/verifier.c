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

static VALUE mDKIM, cDKIMVerifier, cDKIMResolver, cDKIMSignature, cTime;

// feed data to the dkim context
VALUE verifier_feed(VALUE obj, VALUE text)
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
VALUE verifier_finish(VALUE obj)
{
    // get the dkim context
    pdkim_ctx *ctx;
    Data_Get_Struct(obj, pdkim_ctx, ctx);
    
    // finish up the call
    pdkim_signature *signatures;
    
    int result = pdkim_feed_finish(ctx,&signatures);
    if ( result == PDKIM_OK ) {
        
        // create an array to hold the new signatures
        VALUE rsigs = rb_ary_new();
        
        // step through the list of signatues
        while (signatures != NULL) {
          
          // add each signature to the array
          rb_ary_push(rsigs, new_signature(signatures));

          // look for another signature
          signatures = signatures->next;
        }
        return rsigs;
    }
    else {
        rb_raise(rb_eRuntimeError, "error finishing signature: %d", result);
    }
}

// looks up the DNS record given
int query_dns_txt(char *name, char *answer) {
    // call the lookup_record function from the ruby resolver lib
    // this lets us customize resolution in ruby
    VALUE record = rb_funcall(cDKIMResolver, rb_intern("lookup_record"), 1, rb_str_new2(name));

    // copy the results to the context
    strcpy(answer, StringValuePtr(record));
    return PDKIM_OK;
}

// frees the class when we're done
static void verifier_free(void *ctx) {
  pdkim_free_ctx(ctx);
}

// initializes the verifier object
static VALUE verifier_init(VALUE self) {
  return self;
}

// allocates a new dkim verifier object
VALUE verifier_new(VALUE class)
{
    // create the DKIM context
    pdkim_ctx *ctx = pdkim_init_verify(PDKIM_INPUT_NORMAL,
                              &query_dns_txt
                             );
    
    // wrap it into our class
    VALUE obj = Data_Wrap_Struct(class, 0, verifier_free, ctx);
    
    // initialize and return it
    rb_obj_call_init(obj, 0, 0);
    return obj;
}

// sends debugging output to the given file
VALUE verifier_debug(VALUE obj, VALUE file)
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
    return Qnil;
}

// defines the new ruby class and hooks up the proper methods
void Init_verifier() {
  mDKIM = rb_define_module("DKIM");
  cDKIMVerifier = rb_define_class_under(mDKIM, "Verifier", rb_cObject);
  cDKIMResolver = rb_define_class_under(mDKIM, "Resolver", rb_cObject);
  cDKIMSignature = rb_define_class_under(mDKIM, "Signature", rb_cObject);
  cTime = rb_define_class("Time", rb_cObject);
  rb_define_singleton_method(cDKIMVerifier, "new", verifier_new, 0);
  rb_define_method(cDKIMVerifier, "initialize", verifier_init, 0);
  rb_define_method(cDKIMVerifier, "feed", verifier_feed, 1);
  rb_define_method(cDKIMVerifier, "finish", verifier_finish, 0);
  rb_define_method(cDKIMVerifier, "debug", verifier_debug, 1);
}

