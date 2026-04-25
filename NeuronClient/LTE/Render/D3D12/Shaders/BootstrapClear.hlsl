struct VertexOutput {
  float4 position : SV_Position;
};

VertexOutput VSMain(uint vertexID : SV_VertexID) {
  static const float2 positions[3] = {
    float2(-1.0f, -1.0f),
    float2(-1.0f,  3.0f),
    float2( 3.0f, -1.0f),
  };

  VertexOutput output;
  output.position = float4(positions[vertexID], 0.0f, 1.0f);
  return output;
}

float4 PSMain(VertexOutput input) : SV_Target0 {
  return float4(0.02f, 0.04f, 0.08f, 1.0f);
}