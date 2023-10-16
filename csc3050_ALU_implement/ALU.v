module alu(i_datain,gr1, gr2, c, flags);

output signed[31:0] c;
output [2:0]flags;

input signed[31:0] i_datain, gr1, gr2;

reg ALUsrc;
reg[4:0] ALUctr;
reg[5:0] opcode, func;
reg [4:0] shamt;
reg [4:0] rs, rt;
reg fg;
reg signed[31:0] reg_A, reg_B, reg_C;
reg[31:0] reg_Au, reg_Bu, im;
reg zero, overflow, neg;

parameter MSB = 31;


always @(i_datain,gr1, gr2)
begin

    opcode = i_datain[31:26];
    func = i_datain[5:0];
    shamt = i_datain[10:6];
    im = {{16{i_datain[15]}}, i_datain[15:0]};
    rs = i_datain[25:21];
    rt = i_datain[20:16];
    if(rs == 0)
        begin
            reg_A = gr1;
            reg_Au = gr1;
        end
    if(rs == 1)
        begin
            reg_A = gr2;
            reg_Au = gr2;
        end    
    if(rt == 0)
        begin
            reg_B = gr1;
            reg_Bu = gr1;
        end
    if(rt == 1)
        begin
            reg_B = gr2;
            reg_Bu = gr2;
        end

    ALUctr = 5'b1_1111;

    case(opcode)

        6'h0:
        begin
            ALUsrc = 1'b0;
            case (func)
                6'b100000: 
                // add
                begin
                    ALUctr = 5'b00000;
                end

                6'b100001: 
                // addu
                    ALUctr = 5'b00001;
                
                6'b100010: 
                // sub
                    ALUctr = 5'b00010;
                
                6'b100011: 
                // subu
                    ALUctr = 5'b00011;
                
                6'b000010: 
                // srl
                    ALUctr = 5'b00100;
                
                6'b000110: 
                // srlv
                    ALUctr = 5'b00101;

                6'b000011: 
                // sra
                    ALUctr = 5'b00110;

                6'b000111: 
                // srav
                    ALUctr = 5'b00111;

                6'b100100: 
                // and
                    ALUctr = 5'b01000;

                6'b100111: 
                // nor
                    ALUctr = 5'b01001;
                
                6'b100101: 
                // or
                    ALUctr = 5'b01010;
                
                6'b100110: 
                // xor
                    ALUctr = 5'b01011;
                
                6'b101010: 
                // slt
                    ALUctr = 5'b01100;
                
                6'b101011: 
                // sltu
                    ALUctr = 5'b01101;
                
                6'b000000: 
                // sll
                    ALUctr = 5'b10000;
                
                6'b000100: 
                // sllv
                    ALUctr = 5'b10001;
                
                default:
                    ALUctr = 5'b11111;
            endcase

        end

        6'he:
        // xori
        begin
            ALUctr = 5'b0_1011;
            ALUsrc = 1'b1;
            im = {{16{1'b0}}, i_datain[15:0]};
        end

        
        6'hc:
        // andi
        begin
            ALUctr = 5'b0_1000;
            ALUsrc = 1'b1;
            im = {{16{1'b0}}, i_datain[15:0]};
        end

        6'hd:
        // ori
        begin
            im = {{16{1'b0}}, i_datain[15:0]};
            ALUctr = 5'b0_1010;
            ALUsrc = 1'b1;
        end

        6'h9:
        // addiu
        begin
            ALUctr = 5'b0_0001;
            ALUsrc = 1'b1;
        end

        6'h8:
        // addi
        begin
            ALUctr = 5'b0_0000;
            ALUsrc = 1'b1;
        end

        6'h4:
        // beq
        begin
            ALUctr = 5'b0_1111;
            ALUsrc = 1'b0;
        end

        6'h5:
        // bnq
        begin
            ALUctr = 5'b0_1111;
            ALUsrc = 1'b0;
        end

        6'ha:
        // slti
        begin
            ALUctr = 5'b0_1100;
            ALUsrc = 1'b1;
        end

        6'hb:
        // sltiu
        begin
            ALUctr = 5'b0_1101;
            ALUsrc = 1'b1;
        end

        6'h23:
        // lw
        begin
            ALUctr = 5'b0_1110;
            ALUsrc = 1'b1;
        end

        6'h2b:
        // sw
        begin
            ALUctr = 5'b0_1110;
            ALUsrc = 1'b1;
        end
            
    endcase

    if(ALUsrc==1'b1)
        begin
            reg_B = im;
            reg_Bu = im;
        end
    

    overflow = 1'b0;
    neg = 1'b0;
    case (ALUctr)
        //add, addi 
        5'b0_0000:
        begin
            {fg, reg_C} = {reg_A[MSB], reg_A} + {reg_B[MSB], reg_B};
            neg = reg_C[MSB];
            overflow = fg ^ reg_C[MSB];
            // $monitor("%h:%h:%h:%h:%h:%h:%h",reg_A[MSB], reg_B[MSB],reg_A,reg_B, fg, reg_C[MSB],reg_C);
        end 

        // addu, addiu
        5'b0_0001:
        begin
            reg_C = reg_Au + reg_Bu;
        end

        // sub
        5'b0_0010:
        begin
            {fg, reg_C} = {reg_A[MSB], reg_A} - {reg_B[MSB], reg_B};
            neg = reg_C[MSB];
            overflow = fg ^ reg_C[MSB];
        end

        // subu
        5'b0_0011:
        begin
            reg_C = reg_Au - reg_Bu;
        end 

        // srl
        5'b0_0100:
        begin
            reg_A = {{27{1'b0}}, shamt};
            reg_C = reg_B >> reg_A;
            neg = reg_C[MSB];
        end

        //srlv
        5'b0_0101:
        begin
            reg_C = reg_B >> reg_A;
            neg = reg_C[MSB];
        end

        //sra
        5'b0_0110:
        begin
            reg_A = {{27{1'b0}}, shamt};
            reg_C = reg_B >>> reg_A; //signed
            neg = reg_C[MSB];
        end

        //srav
        5'b0_0111:
        begin
            reg_C = reg_B >>> reg_A; //signed
            neg = reg_C[MSB];
        end

        //and
        5'b0_1000:
        begin
            reg_C = reg_A & reg_B;
            neg = reg_C[MSB];
        end

        //nor
        5'b0_1001:
        begin
            reg_C = ~(reg_A | reg_B);
            neg = reg_C[MSB];
        end

        //or
        5'b0_1010:
        begin
            reg_C = reg_A | reg_B;
            neg = reg_C[MSB];
        end

        //xor
        5'b0_1011:
        begin
            reg_C = reg_A ^ reg_B;
            neg = reg_C[MSB];
        end

        //slt
        5'b0_1100:
        begin
            reg_C = reg_A < reg_B;
            neg = reg_C > 0;
        end

        //sltu
        5'b0_1101:
        begin
            reg_C = reg_Au < reg_Bu;
            neg = 1'b0;
        end

        //lw, sw
        5'b0_1110:
        begin
            reg_C = reg_A + reg_B;
        end

        //beq, bne
        5'b0_1111:
        begin
            {fg, reg_C} = {reg_A[MSB], reg_A} - {reg_B[MSB], reg_B};
            neg = reg_C[MSB];
        end
        
        //sll
        5'b1_0000:
        begin
            reg_A = {{27{1'b0}}, shamt};
            reg_C = reg_B << reg_A;
            neg = reg_C[MSB];
        end

        //sllv
        5'b1_0001:
        begin
            reg_C = reg_B << reg_A;
            neg = reg_C[MSB];
        end

        
        default: 
            begin
                $display("invalid input/Wrong instruction");
            end
    endcase
    zero = reg_C ? 0 : 1;
end

assign c = reg_C[31:0];
assign flags = {zero,neg,overflow};
endmodule