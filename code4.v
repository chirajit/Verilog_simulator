module main ;
	wire r,x;
	reg y, z, qq;
	assign r = y;
		initial begin
			$monitor(r, y, z, qq, x);
			y = 1'b1;
			#1
			y = 1'b0;
			z = 1'b0;
			$display(y,z);
			#1
			y = 1'b1;
			z = y|qq;
		end
	always @(r, x, y) begin
		y = 1'b0;
		#1 z= 1'b1;
	end
	always @(r, x) begin
		qq = 1'b1;
	end
endmodule
