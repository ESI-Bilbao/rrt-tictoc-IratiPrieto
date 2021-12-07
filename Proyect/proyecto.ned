import ned.DatarateChannel;

simple Tic
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
}

simple Toc
{
    parameters:
        @display("i=block/process");
    gates:
        input in;
        output out;
}



network Tictoc
{
    @display("bgb=393,242");
    types:
        channel Channel extends ned.DatarateChannel
        {
            datarate = 9600000bps;
            delay = 100us;
            per = 0.1;
        }
    submodules:
        tic: Tic {
            parameters:
                @display("i=,cyan;p=227,175");
        }
        toc: Toc {
            parameters:
                @display("i=,red");
        }
    connections:
        tic.out --> Channel --> toc.in;
        tic.in <-- Channel <-- toc.out;
}
