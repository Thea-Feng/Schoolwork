`timescale 100fs/100fs
module CPU_test;
    reg clock;
    reg start;
    reg [31:0] read_in [0:511];
    reg [16383:0] i_datain;
    integer t;
    // #10clock = ~clock;
    CPU test(.clock(clock), .start(start));
    reg a, b, c;
    
    integer i, file, fd, code;
    initial begin
        // for(i=0;i<100;i++) begin
        //     #10 clock = ~clock;
        // end
        
        forever begin
            #10 clock = ~clock;
        end
    end

    initial
    begin
        clock = 0;
        start = 0;
        a = 1'bx;
    end
    
    always @(*) begin
        if (test.TerminateW == 1'b1) begin
            $display("Finished");
            file = $fopen("DATA_RAM","w");
            for (i = 0; i < 2048; i = i+4) begin
                $display("%x \t %b \t", 
                i,  {test.DATA_RAM[i],test.DATA_RAM[i+1], test.DATA_RAM[i+2], test.DATA_RAM[i+3]});
                $fdisplayb(file, {test.DATA_RAM[i],test.DATA_RAM[i+1], test.DATA_RAM[i+2], test.DATA_RAM[i+3] });
            end
            $fclose(file);
            $finish;
        end 
    end

endmodule