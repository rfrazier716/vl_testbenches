`default_nettype none //flags an error if you haven't defined a wire

module d_flip_flop(
    input wire i_clk, //Clock wire
    input wire i_data_in, //Data in 
    output reg o_q_out, //output
    output wire o_nq_out // inverted output
);

always@(posedge i_clk) begin
    o_q_out<=i_data_in;
end

assign o_nq_out = !o_q_out;

endmodule; 
