// `timescale 1ns / 1ps
module CPU(clock, start);
    input clock;
    input start;
    integer i, j;
    integer cnt;
    reg TerminateD, TerminateM, TerminateE, TerminateW;
    reg [31:0] gr[31:0];
    reg [7:0] RAM[0:2048-1]; //instruction memory
    reg [7:0] DATA_RAM[0:2048-1]; // data memory
    reg [31:0] indata[0:511];

    //IF
    reg [31:0] PC;
    reg [31:0] PCF;
    reg [31:0] PCPlus4F;
    reg [31:0] RDF;
    reg [31:0] PCBranchD;


    //ID
    reg [31:0] InstrD;
    reg [31:0] PCPlus4D;
    reg RegWriteD;
    reg MemtoRegD;
    reg MemWriteD;
    reg MemReadD;
    reg BranchD;
    reg [4:0] A1;
    reg [4:0] A2;
    reg [4:0] A3;
    reg [31:0] WD3;
    reg [31:0] RD1;
    reg [31:0] RD2;
    reg [4:0] RsD;
    reg [4:0] RtD;
    reg [4:0] RdD;
    reg [4:0] ALUControlD;
    reg ALuSrcD;
    reg RegDstD;
    reg JumpD;
    reg JalD;
    reg PCSrcD;
    reg [1:0] ForwardAD;
    reg [1:0] ForwardBD;
    reg [31:0] e1;
    reg [31:0] e2;
    reg [25:0] AddressD;
    reg [31:0] ShamtImmD;
    reg [31:0] SignImmD;


    //EX
    reg RegWriteE;
    reg MemtoRegE;
    reg MemWriteE;
    reg [4:0] ALUControlE;
    reg ALuSrcE;
    reg RegDstE;
    // reg PCSrcE;
    reg JrD;
    reg JalE;
    reg MemReadE;
    reg [31:0] ShamtImmE;
    reg [4:0] RtE;
    reg [4:0] RdE;
    reg [4:0] RsE;
    reg [4:0] WriteRegE;
    reg [31:0] SignImmE;
    reg [31:0] PCPlus4E;
    reg [31:0] SrcAE;
    reg [31:0] SrcBE;
    reg [31:0] WriteDataE;
    reg [31:0] ALUOutE;
    reg Zero;
    reg neg;
    reg overflow;

    // hazard unit
    reg [1:0] ForwardAE;
    reg [1:0] ForwardBE;
    reg Stall;

    //MEM
    reg RegWriteM;
    reg MemtoRegM;
    reg MemWriteM;
    reg MemReadM;
    reg [31:0] WD;
    reg [31:0] A;
    reg [31:0] ALUOutM;
    reg [4:0] WriteRegM;
    reg [31:0] WriteDataM;

    //WB
    reg RegWriteW;
    reg MemtoRegW;
    reg [31:0] ReadDataW;
    reg [31:0] ALUOutW;
    reg [31:0] ResultW;
    reg [4:0] WriteRegW;

always @(start)
    begin
        PC = 32'h0000_0000;
        JumpD = 1'b0;
        AddressD = 1'b0;
        JalD = 1'b0;
        JrD = 1'b0;
        PCPlus4F = 32'h0000_0000;
        ForwardAE = 2'b00;
        ForwardBE = 2'b00;
        Stall = 1'b0;
        TerminateD = 1'b0;
        RegWriteM = 1'b0;
        RegWriteW = 1'b0;
        WriteRegM = 5'b00000;
        WriteRegW = 5'b00000;
        PCSrcD = 1'b0;

        
        
        $readmemb("instructions.bin",indata);
        // $readmemb("machine_code4.txt",indata);

        {RAM[0], RAM[1], RAM[2], RAM[3]} = indata[0];
        j = 4;
        cnt = 0;
        for(i=1; indata[i-1] != 32'hffff_ffff; i++)
            begin
                {RAM[j], RAM[j+1], RAM[j+2], RAM[j+3]} = indata[i];
                // $display("%b",indata[i]);
                j += 4;
            end
        for(i=0;i<2048;i++)
            DATA_RAM[i] = 7'b000_0000;
        for(i=0;i<32;i++)
            gr[i] = 32'h0000_0000;
        // $display("This is beginning");
    end

// update PC
always @(PCSrcD, JrD, PCPlus4F, JumpD, AddressD)
    begin
        
        if(JumpD == 1'b1 || JalD == 1'b1)
            PC = AddressD;
        else if(JrD == 1'b1)
            PC = e1;
        else 
            begin
                case(PCSrcD)
                    1'b1:
                        PC = PCBranchD;
                    default:
                        PC = PCPlus4F;
                endcase
            end
    end

// Fetch part
always @(posedge clock)
    begin
        if(Stall !=1'b1)
            begin
                PCF <= PC;
            end
        // $display("\n \nThis is %d turn", cnt);
        // $display("CLK! Fetch part: En is %d and PCF is %b\t PC is %b",Stall, PCF,PC);
    end

// IF
always @(PCF)
    begin
        RDF = {RAM[PCF], RAM[PCF+1], RAM[PCF+2], RAM[PCF+3]};
        PCPlus4F = PCF + 4;
        // $display("RDF is %b", RDF);
    end

// IF/ID
always @(posedge clock)
    begin
        if(PCSrcD == 1'b1 || JumpD == 1'b1 || JalD == 1'b1 || JrD == 1'b1)
            begin
                InstrD <= 32'h0000_0000;
                PCPlus4D <= 32'h0000_0000;
            end
        else if(Stall != 1'b1)
            begin
                InstrD <= RDF;
                PCPlus4D <= PCPlus4F;
            end
        // $display("CLK! IF/DF: InstrD is %b", InstrD);
    end

//ID
always @(InstrD, WriteRegW, ResultW)
    begin
        if(InstrD[31:0] == 32'hffff_ffff)
        begin
            TerminateD = 1'b1;
            InstrD = 32'h0000_0000;
        end
        
        // data field
        A1 = InstrD[25:21];
        A2 = InstrD[20:16];
        RsD = InstrD[25:21];
        RtD = InstrD[20:16];
        RdD = InstrD[15:11];
        SignImmD = {{16{InstrD[15]}}, InstrD[15:0]};
        PCBranchD = PCPlus4D + {SignImmD[30:0], 2'b00};
        ShamtImmD = {{27{1'b0}}, InstrD[10:6]};
        AddressD = {PCPlus4D[31:28], InstrD[25:0], 2'b00};

        //control unit
        RegDstD = 1'b0;
        ALuSrcD = 1'b0;
        MemtoRegD = 1'b0;
        RegWriteD = 1'b0;
        MemWriteD = 1'b0;
        MemReadD = 1'b0;
        BranchD = 1'b0;
        JalD = 1'b0;
        JumpD = 1'b0;

        case(InstrD[31:26])
            6'b00_0000: // R-type
            begin
                RegDstD = 1'b1;
                RegWriteD = 1'b1;
                case (InstrD[5:0])
                    //jr
                    6'b001000:
                    begin
                        RegWriteD = 1'b0;
                        ALUControlD = 5'b10100;
    
                    end

                    6'b100000: 
                    // add
                        ALUControlD = 5'b00000;

                    6'b100001: 
                    // addu
                        ALUControlD = 5'b00001;
                    
                    6'b100010: 
                    // sub
                        ALUControlD = 5'b00010;
                    
                    6'b100011: 
                    // subu
                        ALUControlD = 5'b00011;
                    
                    6'b000010: 
                    // srl
                        ALUControlD = 5'b00100;
                    
                    6'b000110: 
                    // srlv
                        ALUControlD = 5'b00101;

                    6'b000011: 
                    // sra
                        ALUControlD = 5'b00110;

                    6'b000111: 
                    // srav
                        ALUControlD = 5'b00111;

                    6'b100100: 
                    // and
                        ALUControlD = 5'b01000;

                    6'b100111: 
                    // nor
                        ALUControlD = 5'b01001;
                    
                    6'b100101: 
                    // or
                        ALUControlD = 5'b01010;
                    
                    6'b100110: 
                    // xor
                        ALUControlD = 5'b01011;
                    
                    6'b101010: 
                    // slt
                        ALUControlD = 5'b01100;
                    
                    6'b000000: 
                    // sll
                        ALUControlD = 5'b10000;
                    
                    6'b000100: 
                    // sllv
                        ALUControlD = 5'b10001;
                    
                    default:
                        ALUControlD = 5'b11111;
                
                endcase
            end

            //addi
            6'h8:
            begin
                ALuSrcD = 1'b1;
                RegWriteD = 1'b1;
                ALUControlD = 5'b0_0000;
            end

            //addiu
            6'h9:
            begin
                ALuSrcD = 1'b1;
                RegWriteD = 1'b1;
                ALUControlD = 5'b0_0000;
            end

            //andi
            6'hc:
            begin
                ALuSrcD = 1'b1;
                RegWriteD = 1'b1;
                ALUControlD = 5'b1_0011;
            end

            //ori
            6'hd:
            begin
                ALuSrcD = 1'b1;
                RegWriteD = 1'b1;
                ALUControlD = 5'b0_1110;
            end

            //xori
            6'he:
            begin
                ALUControlD = 5'b1_0010;
                ALuSrcD = 1'b1;
                RegWriteD = 1'b1;
            end

            //beq
            6'h4:
            begin
                BranchD = 1'b1;
                ALUControlD = 5'b0_0010;
            end

            //bne
            6'h5:
            begin
                BranchD = 1'b1;
                ALUControlD = 5'b0_1111;
            end

            //lw
            6'h23:
            begin
                ALuSrcD = 1'b1;
                MemtoRegD = 1'b1;
                RegWriteD = 1'b1;
                ALUControlD = 5'b0_0000;
                MemReadD = 1'b1;
            end

            //sw
            6'h2b:
            begin
                ALuSrcD = 1'b1;
                MemtoRegD = 1'b1;
                MemWriteD = 1'b1;
                ALUControlD = 5'b0_0000;
            end

            //j
            6'h2:
                JumpD = 1'b1;

            //jal
            6'h3:
            begin
                ALUControlD = 5'b0_1101;
                RegWriteD = 1'b1;
                JalD = 1'b1;
            end
        endcase
        // $display("ALUControlD: %b \t ALuSrcD: %b \t InstrD: %b", ALUControlD, ALuSrcD, InstrD);
    end
reg fg;
integer MSB = 31;
reg signed [31:0] reg_A;
reg signed [31:0] reg_B;
reg [31:0] reg_Au;
reg [31:0] reg_Bu;
// Write register before read, using negedge
always @(negedge clock)
    begin
        A3 = WriteRegW;
        WD3 = ResultW;
        if(RegWriteW)
            gr[A3] = WD3;
    end
// data hazard & EX
always @(negedge clock)
    begin
        //hazard unit
        WriteRegE = RegDstE ? RdE : RtE;
        // if((RegWriteM!=1'bx' )
        ForwardAE = ((RegWriteM==1'b1)&&(WriteRegM!=5'b0)&&(WriteRegM==RsE)) ? 2'b10 : 2'b00;
        ForwardBE = ((RegWriteM==1'b1)&&(WriteRegM!=5'b0)&&(WriteRegM==RtE)) ? 2'b10 : 2'b00;
        Stall = (MemReadE==1'b1&&((RtE==RtD)||(RtE==RsD))) ? 1'b1 : 1'b0;   
        if(ForwardAE!=2'b10)
            ForwardAE = ((RegWriteW==1'b1)&&(WriteRegW!=5'b0)&&(WriteRegW==RsE)) ? 2'b01 : 2'b00;
        if(ForwardBE!=2'b10)
            ForwardBE = ((RegWriteW==1'b1)&&(WriteRegW!=5'b0)&&(WriteRegW==RtE)) ? 2'b01 : 2'b00;
        // $display("AE %b, BE %b", ForwardAE, ForwardBE);


        case (ForwardAE)
            2'b10:
                SrcAE = ALUOutM;
            2'b01: 
                SrcAE = ResultW;
            default:
                SrcAE = RD1;
        endcase

        case (ForwardBE)
            2'b10:
                SrcBE = ALUOutM;
            2'b01: 
                SrcBE = ResultW;
            default:
                SrcBE = RD2;
        endcase

        WriteDataE = SrcBE;

        if(ALuSrcE == 1'b1)
            begin
                SrcBE = SignImmE;
            end

        if(ALUControlE == 5'b0_1101)
            begin
                WriteRegE = 32'd31;
                SrcAE = PCPlus4E;
                SrcBE = 32'b0;
            end
        reg_A = SrcAE;
        reg_B = SrcBE;
        
        overflow = 1'b0;
        neg = 1'b0;
        Zero = 1'b0;

        reg_Au = SrcAE;
        reg_Bu = SrcBE;
        case (ALUControlE)
            5'b00000:
            //add addi lw, sw
            begin
                {fg, ALUOutE} = {reg_A[MSB], reg_A} + {reg_B[MSB], reg_B};
                neg = ALUOutE[MSB];
                overflow = fg ^ ALUOutE[MSB];
            end

            // addu, addiu
            5'b0_0001:
            begin
                ALUOutE = reg_Au + reg_Bu;
            end

            // sub, beq
            5'b0_0010:
            begin
                {fg, ALUOutE} = {reg_A[MSB], reg_A} - {reg_B[MSB], reg_B};
                neg = ALUOutE[MSB];
                overflow = fg ^ ALUOutE[MSB];
            end

            // subu
            5'b0_0011:
            begin
                ALUOutE = reg_Au - reg_Bu;
            end 
            
            //and
            5'b0_1000:
            begin
                ALUOutE = reg_A & reg_B;
                neg = ALUOutE[MSB];
            end

            //andi
            5'b1_0011:
            begin
                reg_B = {{16{1'b0}}, reg_B[15:0]};
                ALUOutE = reg_A & reg_B;
                neg = ALUOutE[MSB];
            end

            //nor
            5'b0_1001:
            begin
                ALUOutE = ~(reg_A | reg_B);
                neg = ALUOutE[MSB];
            end

            //or
            5'b0_1010:
            begin
                ALUOutE = reg_A | reg_B;
                neg = ALUOutE[MSB];
            end

            //xor
            5'b0_1011:
            begin
                ALUOutE = reg_A ^ reg_B;
                neg = ALUOutE[MSB];
                // $display("xor: %h \t %h \t %h",ALUOutE, reg_A, reg_B);
            end

            //xori
            5'b1_0010:
            begin
                reg_B = {{16{1'b0}}, reg_B[15:0]};
                ALUOutE = reg_A ^ reg_B;
                neg = ALUOutE[MSB];
            end

            // srl
            5'b0_0100:
            begin
                reg_A = ShamtImmE;
                ALUOutE = reg_B >> reg_A;
                neg = ALUOutE[MSB];
            end

            //srlv
            5'b0_0101:
            begin
                ALUOutE = reg_B >> reg_A;
                neg = ALUOutE[MSB];
            end

            //sra
            5'b0_0110:
            begin
                reg_A = ShamtImmE;
                ALUOutE = reg_B >>> reg_A; //signed
                neg = ALUOutE[MSB];
            end

            //srav
            5'b0_0111:
            begin
                ALUOutE = reg_B >>> reg_A; //signed
                neg = ALUOutE[MSB];
            end

            //slt
            5'b01100:
            begin
                ALUOutE = reg_A < reg_B;
                neg = ALUOutE > 0;
            end

            //sll
            5'b1_0000:
            begin
                reg_A = ShamtImmE;
                ALUOutE = reg_B << reg_A;
                neg = ALUOutE[MSB];
            end

            //sllv
            5'b1_0001:
            begin
                ALUOutE = reg_B << reg_A;
                neg = ALUOutE[MSB];
            end

            //bne
            5'b0_1111:
            begin
                {fg, ALUOutE} = {reg_A[MSB], reg_A} - {reg_B[MSB], reg_B};
                neg = ALUOutE[MSB];
            end

            //ori
            5'b0_1110:
            begin
                reg_B = {{16{1'b0}}, reg_B[15:0]};
                ALUOutE = reg_A | reg_B;
                neg = ALUOutE[MSB];
            end
            
            //jal  ??? // combine with add or delete JalE
            5'b0_1101:
            begin
                ALUOutE = reg_A + reg_B;
            end


        endcase
        Zero = ALUOutE ? 0 : 1;
        if(ALUControlE == 5'b0_1111)
            Zero = Zero ^ 1;
            
        // PCSrcE = (BranchE==1'b1)&&(Zero==1'b1);
        // $display("ALUControlE is %b \t ALUOutE is %b \t WriteRegE is %b \t", ALUControlE, ALUOutE, WriteRegE );
        #2 cnt += 1;
    end
always @(negedge clock)       
begin
    if(Stall != 1'b1)
    begin 
        #2
        ForwardAD = ((RegWriteE==1'b1)&&(WriteRegE!=5'b0)&&(WriteRegE==RsD)) ? 2'b10 : 2'b00;
        ForwardBD = ((RegWriteE==1'b1)&&(WriteRegE!=5'b0)&&(WriteRegE==RtD)) ? 2'b10 : 2'b00;
        if(ForwardAD!=2'b10)
            ForwardAD = ((RegWriteM==1'b1)&&(WriteRegM!=5'b0)&&(WriteRegM==RsD)) ? 2'b01 : 2'b00;
        if(ForwardBE!=2'b10)
            ForwardBD = ((RegWriteM==1'b1)&&(WriteRegM!=5'b0)&&(WriteRegM==RtD)) ? 2'b01 : 2'b00;
        // $display("AE %b, BE %b", ForwardAE, ForwardBE);
        case (ForwardAD)
            2'b10:
                e1 = ALUOutE;
            2'b01: 
                e1 = MemReadM ? {DATA_RAM[ALUOutM],DATA_RAM[ALUOutM+1],DATA_RAM[ALUOutM+2],DATA_RAM[ALUOutM+3]} : ALUOutM;
            default:
                e1 = gr[A1];
        endcase

        case (ForwardBD)
            2'b10:
                e2 = ALUOutE;
            2'b01: 
                e2 = MemReadD ? {DATA_RAM[ALUOutM],DATA_RAM[ALUOutM+1],DATA_RAM[ALUOutM+2],DATA_RAM[ALUOutM+3]} : ALUOutM;
            default:
                e2 = gr[A2];
        endcase
        if((ALUControlD == 5'b0_0010) && BranchD && (e1 == e2))
            PCSrcD = 1'b1;
        else if((ALUControlD == 5'b0_1111) && BranchD && (e1 != e2)) 
            PCSrcD = 1'b1;
        else PCSrcD = 1'b0;

        if(ALUControlD == 5'b10100)
            begin

                JrD = 1'b1;
            end
        else JrD = 1'b0;
    end
end

// ID/EX
always @(posedge clock)
    begin
        if(Stall == 1'b1) //?
            begin  //flush
                RegWriteE <= 1'b0;
                TerminateE <= 1'b0;
                MemtoRegE <= 1'b0;
                MemWriteE <= 1'b0;
                ALUControlE <= 5'b0;
                ALuSrcE <= 1'b0;
                RegDstE <= 1'b0;
                WriteDataE <= 5'b0;
                RtE <= 5'b0;
                RdE <= 5'b0;
                RsE <= 5'b0;
                SignImmE <= 32'b0;
                ShamtImmE <= 32'b0;
                MemReadE <= 1'b0;
                PCPlus4E <= 32'b0;
                RD1 <= 32'b0;
                RD2 <= 32'b0;
            end
        else
            begin     
                TerminateE <= TerminateD;
                RegWriteE <= RegWriteD;
                MemtoRegE <= MemtoRegD;
                MemWriteE <= MemWriteD;
                MemReadE <= MemReadD;
                ALUControlE <= ALUControlD;
                ALuSrcE <= ALuSrcD;
                RegDstE <= RegDstD;
                RtE <= RtD;
                RdE <= RdD;
                RsE <= RsD;
                SignImmE <= SignImmD;
                PCPlus4E <= PCPlus4D;
                ShamtImmE <= ShamtImmD;
                RD1 <= gr[A1];
                RD2 <= gr[A2];
            end
        // $display("CLK! ID/EX:");      
    end





// EX/MEM
always @(posedge clock)
    begin
        RegWriteM <= RegWriteE;
        TerminateM <= TerminateE;
        MemtoRegM <= MemtoRegE;
        MemWriteM <= MemWriteE;
        MemReadM <= MemReadE;
        ALUOutM <= ALUOutE;
        WriteRegM <= WriteRegE;
        WriteDataM <= WriteDataE;
        // $display("CLK! EX/MEM: ALUOutM is %b", ALUOutM);
    end

// MEM
always @(ALUOutM, WriteDataM)
    begin
        A = ALUOutM;
        WD = WriteDataM; 
    end

always @(posedge clock)
    begin
        if(MemWriteM)
            begin
                {DATA_RAM[ALUOutM],DATA_RAM[ALUOutM+1],DATA_RAM[ALUOutM+2],DATA_RAM[ALUOutM+3]} <= WD;
                ReadDataW <= WD;
                // $display("Write, ALUOutM %h, WD %h", ALUOutM, WD);
            end
        else
            ReadDataW <= {DATA_RAM[ALUOutM],DATA_RAM[ALUOutM+1],DATA_RAM[ALUOutM+2],DATA_RAM[ALUOutM+3]};
        // $display("CLK! DM: ReadDataW is %b", ReadDataW);
    end

// MEM/WB
always @(posedge clock)
    begin
        RegWriteW <= RegWriteM;
        TerminateW <= TerminateM;
        MemtoRegW <= MemtoRegM;
        ALUOutW <= ALUOutM;
        WriteRegW <= WriteRegM;
        // $display("CLK! MEM/WB: ReadDataW is %b", ReadDataW);
    end

// WB
always @(ALUOutW, ReadDataW, MemtoRegW)
    begin
        ResultW = MemtoRegW ? ReadDataW : ALUOutW;
        // $display("ResultW is %b", ResultW);
    end

//debug part
// always@(cnt)begin
//     $display("\n\nThis is %d cycle", cnt);
    // $display("IF part\n PCSrD %d PC %h \t PCF %h \t RDF %b \t", PCSrcD, PC, PCF, RDF);
    // $display("ID part\n InstrD %b \t ALUControlD %b\t BranchD %b PCBranchD %h JrD%h", InstrD, ALUControlD, BranchD, PCBranchD,JrD);
    // $display("e1 %h \t e2 %h \t MemtoRegM %b WD3 %b ResultW %b ReadDataW %b", e1,e2, MemtoRegM, WD3, ResultW, ReadDataW);
    // $display("EX part\n SrcAE %h \t SrcBE %h \t WriteDataE %h \t ALUControlE %h\t ForwardAD %b ForwardBD %b e1 %h", SrcAE,SrcBE,WriteDataE, ALUControlE, ForwardAD, ForwardBD,e1);
    // $display("Stall %b MemReadE %b RsD %h\t RtD %h\tE %h\t RtE %h\t RdE %h\t WriteRegM %h", Stall,MemReadE, RsD, RtD, RsE, RtE, RdE, WriteRegM);
    // // $display("ALUCtrE %b \t ALUScrE %b \t SignImmE %b \t", ALUControlE, ALuSrcE, SignImmE);
    // $display("MEM Part\n ReadDataW %h WriteRegM %h dizhi %b ALUOutM %h\t", ReadDataW, WriteRegM, {DATA_RAM[ALUOutM],DATA_RAM[ALUOutM+1],DATA_RAM[ALUOutM+2],DATA_RAM[ALUOutM+3]},ALUOutM);
    // // $display("WB part\n MemtoReg %h \t WriteRegW %h \t ResultW %h \t ReadDataW %h", MemtoRegW, WriteRegW, ResultW, ReadDataW);
    // // $display("Terminate check: D %d E %d M %d W %d", TerminateD, TerminateE, TerminateM, TerminateW);
    // $display("result check: ALUOutE %h \t WriteDataE %h \t WriteRegE %h PCSrcD %b", ALUOutE, WriteDataE, WriteRegE, PCSrcD);

    // for(i=0;i<32;i+=4)
    // $display("%d %h \t%d %h \t%d %h \t%d %h",i,gr[i],i+1,gr[i+1],i+2,gr[i+2],i+3,gr[i+3]);
    // $display("DATA_RAM");
    // for(i=0;i<=100;i+=4)
    // $display("%d %h",i/4,{DATA_RAM[i],DATA_RAM[i+1],DATA_RAM[i+2],DATA_RAM[i+3]});
    // end
endmodule