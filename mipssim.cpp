#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip>
#include <bitset>
#include <string>
#include <sstream>
#include <unordered_map>
using namespace std;



struct instruction{
    int addr, v, opcode, rs, rt, rd, shamt, imm, asInt, func, jaddr;
    bool  pbreak;
    unsigned int asUint;
    string out, instr;
    instruction(){ }; // constructor default
    instruction( int i, int addr ){
        asInt = i;
        this->addr = addr;
        asUint = (unsigned int) i;
        out = bitset<32>(asInt).to_string();
	{
            stringstream ss1;
            ss1 << out << "\t" << addr << "\t";
            out = ss1.str(); // places the entire out of ssl into a full string for output
        }
        v = asUint >> 31; // from right to left 31 bits to check 32nd bit for validity
        opcode = asUint >> 26; // checks 6 bits at end for opcode
        rs = asUint <<6 >>27; // from the right 27 bits then to the left of that 8 bits " 27(unicluded) - 6bits = 21(unincluded)" 
        rs = (asUint >> 21) & 0x0000001F;// "WHAT DOES THE LAST PART MEAN"
        rt = asUint <<11 >> 27;
        imm = asInt << 16 >> 16;
	rd = asUint << 16 >> 27;
	func = asUint << 26 >> 26;
	shamt = asUint << 21 >> 27;
	jaddr = asUint << 6 >> 6;
	if(!pbreak){	
        if( v == 0){
            instr = "Invalid Instruction";
            out += instr;
        } else if( opcode == 40 ){
            stringstream ss1;
            ss1 << "ADDI\tR"<< rt << ", R" << rs<<", #" << imm;
            instr = ss1.str();
            out += instr;
        } else if( opcode == 43 ) {
            stringstream ss1;
            ss1 << "SW\tR" << rt << ", "<<imm<< "(R" << rs << ")";
            instr = ss1.str();
            out += instr;
        } else if( opcode == 35){
		stringstream ssl;
		ssl << "LW\tR" << rt << ", " << imm << "(R" << rs << ")";
	       	instr = ssl.str(); // makes the string of this
		out += instr; // concatinates the strint to the out output	
	} else if( opcode == 33){
		stringstream ssl;
		ssl << "BLTZ\tR" << rs << ", #" << imm*4;
		instr = ssl.str();
		out += instr;
	} else if(( opcode == 32 ) && (func == 0)){
        stringstream ssl;
        if((shamt == 0) && (rt == 0) && (rs == 0)){
           ssl << "NOP";
	  instr = ssl.str();
	 out+=instr; 
        }
        else {
		ssl << "SLL\tR" << rd << ", R" << rt << ", #" << shamt;
	    instr = ssl.str();
		out += instr;	
        }
	} else if (( opcode == 32) && ( func == 34)){
		stringstream ssl;
		ssl << "SUB\tR" << rd << ", R" << rs << ", R" << rt;
		instr = ssl.str();
		out += instr;
	} else if ( opcode == 34){ 
		stringstream ssl;
		ssl << "J\t#" << jaddr*4;
		instr = ssl.str();
		out += instr;
	} else if (( opcode == 32 ) && ( func == 32)){
		stringstream ssl;
		ssl << "ADD\tR" << rd << ", R" << rs << ", R" << rt;
		instr = ssl.str();
		out += instr;
	} else if (( opcode == 32 ) && ( func == 13 )){
		stringstream ssl;
		ssl << "BREAK";
		instr = ssl.str();
		out += instr;
		pbreak = true;
	} else if((opcode == 32) && func == 8){ //Jr
        stringstream ssl;
        ssl << "JR\tR" << rs;
        instr = ssl.str();
        out += instr;
    } else if((opcode == 32) && (func == 2)){ // SRL
        stringstream ssl;
        ssl << "SRL\tR" << rd << ", R" << rt << ", #" << shamt;
        instr = ssl.str();
        out += instr;
    } else if ((opcode == 60 ) && (func == 2)) {
        stringstream ssl;
        ssl << "MUL\tR" << rd << ", R" << rs << ", R" << rt;
        instr = ssl.str();
        out += instr;
    } else if ((opcode == 32) && (func == 10)) { 
        stringstream ssl;
        ssl << "MOVZ\tR" << rd << ", R" << rt << ", R" << rt;
        instr = ssl.str();
        out += instr;
    }
	}

	else if(pbreak){
		stringstream ssl;
		ssl << asInt;
		instr = ssl.str();
		out += instr;
	}
	
    }
};

int main( )
{
    char buffer[4];
    int pbreakaddr;
    bool pbreak = false;
    int maxaddr;
    int i;
    char * iPtr;
    iPtr = (char*)(void*) &i;
    int addr = 96;
    int FD = open("test3.bin", O_RDONLY);
    unordered_map< int, instruction> MEM;
    int amt = 4;
    while( amt != 0 )
    {
        amt = read(FD, buffer, 4);

        if( amt == 4)
        {
            iPtr[0] = buffer[3];
            iPtr[1] = buffer[2];
            iPtr[2] = buffer[1];
            iPtr[3] = buffer[0];
            //cout << "i = " <<hex<< i << endl;
            instruction I( i, addr);
            maxaddr = addr;
            if((I.opcode == 32) && (I.func == 13)){
                pbreakaddr = I.addr + 4;
            } 
            cout << I.out << endl;
            //cout << "op code: " << dec << (((unsigned int)i)>>26) << endl;
            //cout << "rs bits: " << ((((unsigned int)i)<<6)>>27) << endl;
            MEM[addr] = I;
            addr += 4;
        }
    }
    // to get the ammount of instructions after the pbreakaddr
    int incr = (maxaddr - pbreakaddr) / 4;
    
    struct processor{
        int PC = 96, cycle = 1, R[32] = {0}, pbreakaddr;

        void print( const instruction  &I ){
            stringstream ss1;
        //     int pbreakaddr;
            ss1 << "====================\ncycle:" << cycle << "\t"
                << PC <<"\t" << I.instr << "\n\nregisters:";
            for( int i = 0; i < 32; i++ ){
                if( i %8 == 0 )
		ss1 << "\nr"<<i<<":";
                ss1 << "\t" << R[i];
                // ss1 << I.addr; = address it is currently on
            }
            cout << ss1.str() << endl;
        }

    };



    //sim!
    int temp = pbreakaddr; 
    processor P;
    int * point;
    while( true ){
        instruction I = MEM[ P.PC ];
        while( I.v == 0 ){ // tests valid bit and skips the first instruction due to it being a zero
            P.PC +=4; // then increments key + 4 making the pc start at 100
            I = MEM[P.PC]; // sets instruction I at value stored in "array" value's key
        }
        if( I.opcode == 40 ){ // ADDI
            P.R[ I.rt ] =  P.R [I.rs]  + I.imm ;
        } else if( I.opcode == 43 ) { // SW 
            MEM[I.imm + P.R[I.rs]].asInt = P.R[I.rt];
        } else if( I.opcode == 35){ // LW 
            P.R[I.rt] = MEM[I.imm + P.R[I.rs]].asInt;
        } else if( I.opcode == 33){ //BLTZ
            if(P.R[I.rs] < 0){
                P.PC = P.PC + I.imm*4;
            }
        } else if(( I.opcode == 32 ) && (I.func == 0)){ // SLL
            if((I.shamt == 0) && (I.rs == 0) && (I.rt == 0)){ // NOP (similar as SLL)
		P.print(I);
		stringstream ssl;
		ssl << "\ndata:";
		for(int r = 0, temp = pbreakaddr; r <= incr; r ++, temp += 4 ){
           		 if(r %8 == 0){
               			 ssl << "\n" << temp;    
            	}
           	 ssl << "\t" <<  MEM[temp].asInt;    
        	}
		P.cycle ++;
		P.PC += 4;
		continue;
            }else{
           P.R[I.rd] = P.R[I.rt] << I.shamt; 
            }
        } else if (( I.opcode == 32) && ( I.func == 34)){ // SUB
            P.R[I.rd] = P.R[I.rs] - P.R[I.rt];
        } else if ( I.opcode == 34){ // J
            P.print( I );
            P.PC = I.jaddr * 4;
            stringstream ssl;
            ssl << "\ndata:";
            for(int r = 0, temp = pbreakaddr; r <= incr; r++, temp += 4){ // print the data secton
                if(r %8 == 0){
                    ssl << "\n" << temp;    
                }
            ssl << "\t" <<  MEM[temp].asInt;    
        }
	    cout << ssl.str() << endl;
	    P.cycle ++;
            continue;
        } else if (( I.opcode == 32 ) && ( I.func == 32)){ // ADD
            P.R[I.rd] = P.R[I.rs] + P.R[I.rt];
        } else if (( I.opcode == 32 ) && ( I.func == 13 )){ // BREAK
            P.print( I );
            stringstream ssl;
            ssl << "\ndata:";
            for(int r = 0, temp = pbreakaddr; r <= incr; r ++, temp += 4 ){
                if(r %8 == 0){
                    ssl << "\n" << temp;    
                }
                ssl << "\t" <<  MEM[temp].asInt;     
            }
            cout << ssl.str() << "\n";
            break;
            false;
        } else if ((I.opcode == 32) && (I.func == 8)){ // JR
	    P.print(I);
	    P.PC = P.R[I.rs];
	    stringstream ssl;
	    ssl << "\ndata:";
	    for(int r = 0; r <= incr; r++, temp += 4){
		if(r % 8 == 0){
		    ssl << "\n" << temp;	
		}
		ssl << "\t" << MEM[temp].asInt;
	    }
	    cout << ssl.str() << "\n";
	    P.cycle ++;
	    continue;
        } else if ((I.opcode == 32) && (I.func == 2)){ // SRL
            P.R[I.rd] = P.R[I.rt] >> I.shamt;
        } else if((I.opcode == 60) && (I.func == 2)){ // MUL
            P.R[I.rd] = P.R[I.rs] * P.R[I.rt];
        } else if((I.opcode == 32) && (I.func == 10)){ // MOVZ
            // not sure how to padd this out
	    P.R[I.rd] = P.R[I.rs] + P.R[I.rt];
        } 


       
        P.print( I ); // prints out the registers and values in them along with instruction
        stringstream ssl;
            ssl << "\ndata:";
        for(int r = 0, temp = pbreakaddr; r <= incr; r ++, temp += 4 ){
            if(r %8 == 0){
                ssl << "\n" << temp;    
            }
            ssl << "\t" <<  MEM[temp].asInt;    
        }
        cout << ssl.str();
        cout << '\n';
        P.PC+=4;
        P.cycle ++ ;
        //if(P.cycle >20) exit(0);// P.cycle > 2 
    }
}

