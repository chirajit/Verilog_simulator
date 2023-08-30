module main;
	wire w1,w2,w3;
	reg r1,r2,r3;
	assign w1 = r1;
	assign w2 = r2;
	initial begin
		$monitor(w1,w2,w3,r1,r2,r3);
		r1 = 1'b0;
		r3 = !r1;
		#2
		r2 = 1'b0 | 1'b1; 
		r3 = (r2|r1)&r3;
	end
	always @(r1) begin
		#2 r2 = r1|r3;
	end
	always @(r2) begin
		#2 r2 = r1|r3;
	end
endmodule