/*The IDL File  --- name IDL.x*/

/*Structure to hold a value and a char*/
struct args{
    float value;
    char operation;
};

/*Programme, version and procedure definition*/
program TRPC{
    version TRPC_VERS{
        float GOTEMP(args)=1;
        float GOHUMI(args)=2;
    } =6;

} = 456123789;
