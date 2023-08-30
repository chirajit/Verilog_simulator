module main ;
wire r,x;
reg y, z;
assign r = y|z;
dut d(r,z);
dut d2(x,r);
initial begin 
$monitor(r,x,y,z);
y = r;
z = 1'b1;
#0
z = 1'b1;
#1 z = 1'b1;
#5 z <= 1'b0;
$display(z);
end

initial begin 
#1;
y = 1'b0;
#6;
y = z;
end

endmodule

module dut (input wire r, output reg o);
	wire p,k;
	reg f,g,t;
	initial begin
	g = 1'b1;
	o = g;
	end
endmodule

101978610947