/*The IDL File  --- name trpc.x*/

/*Structure to hold a value and a char*/
struct args{
    float value;
    char operation;
};

/*Programme, version and procedure definition*/
program TRPC {
    version TRPC_VERS {
        void SETTEMP(args) = 1;
        float GETTEMP(void) = 2;
    } = 6;

} = 456123789;
