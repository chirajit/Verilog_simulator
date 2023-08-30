module main ;
reg a,b;
initial begin
$monitor(a,b);
a = 1'b0;
b = 1'b0;
#4 a = 1'b1;
#4 a = 1'b0;
end
bot  ba (b);
endmodule

module bot(output reg b);
initial begin
$monitor(b);
#3 b = 1'b1;
#3 b = 1'b0;
end
endmodule
