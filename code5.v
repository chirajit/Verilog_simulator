module main ;
wire c;
reg a,b;
assign c = b;
initial begin
$display(a,b,c);
a = 1'b0;
b = 1'b0;
$display(b,c);
#4 a = 1'b1;
#4 a = 1'b0;
end
bot  ba (b);
endmodule

module bot(output reg b);
initial begin
#3 b = 1'b1;
#3 b = 1'b0;
end
endmodule
