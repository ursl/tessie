/* The IDL File  --- name trpc.x */
/* tessie rpc */

/*Structure to hold a value and a char*/
struct args{
    float value;
    char operation;
};

/*Programme, version and procedure definition*/
program TRPC {
    version TRPC_VERS {
        void  SETTEMP(args) = 1;
        float GETTEMP(void) = 2;
        void  STARTFLUSH(void) = 3;
        void  STOPFLUSH(void) = 4;
        float GETHUMIDITY(void) = 5;
        int   GETSTATUS(void) = 6;
    } = 6;

} = 456123789;
