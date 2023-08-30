module main ;
    wire r,x;
    reg y, z, qq;
    assign r = 1'b0;
    assign r = 1'b1;
    initial begin
        $monitor(r, y, z, qq);
        y = 1'b1;
        #1
        y = 1'b0;
    end
    always @(r, x, y) begin
        z = 1'b0;
    end
    always @(x) begin
        qq = 1'b1;
    end
endmodule