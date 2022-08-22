
struct VertexInput
{
	float4 position : POSITION0;
	float2 uv : UV0;
	float4 color : COLOR0; //��������
	float tileMapIdx : INDICES0;
	float tileState : STATE0;
};
// PI
struct PixelInput
{
	float4 position : SV_POSITION; //ȭ����ǥ�� ������
	float2 uv : UV0; //���ε� ��ǥ
	float4 color : COLOR0; //��������
	float tileMapIdx : INDICES0;
	float tileState : STATE0;
};

//������� ũ��� 16byte����� �������Ѵ�.

cbuffer VS_WVP : register(b0)
{
	matrix WVP;
}

cbuffer PS_LIGHT : register(b0)
{
	float2 ScreenPos; //��ũ����ǥ
	float Radius; //������ũ��
	float Select; //���°�
	float4 LightColor; //���� ��
	float4 OutColor; //�ܰ� ��
};

//���ؽ� ���̴�
//��ȯ��  �Լ��̸�(�Ű�����)
// VertexInput(in) ->  VS  -> PixelInput (out)
PixelInput VS(VertexInput input)
{
    //������ȯ�� ���� ������ ��
	PixelInput output;
	output.position = mul(input.position, WVP);
	output.color = input.color;
	output.uv = input.uv;
	output.tileMapIdx = input.tileMapIdx;
	output.tileState = input.tileState;
	return output;
}

Texture2D Texture0 : register(t0);
SamplerState Sampler0 : register(s0);

Texture2D Texture1 : register(t1);
SamplerState Sampler1 : register(s1);

Texture2D Texture2 : register(t2);
SamplerState Sampler2 : register(s2);

Texture2D Texture3 : register(t3);
SamplerState Sampler3 : register(s3);


//�ȼ����̴� ���� �Լ�
float4 PS(PixelInput input) : SV_TARGET //SV_TARGET �� Ÿ���̵� ���� 
{
	float4 TextureColor;
    // ���ε� ��ǥ�� �ؽ��� �ε�
   
    [branch]
	if (input.tileMapIdx == 0.0f)
	{
		TextureColor = Texture0.Sample(Sampler0, input.uv);
	}
	else if (input.tileMapIdx == 1.0f)
	{
		TextureColor = Texture1.Sample(Sampler1, input.uv);
	}
	else if (input.tileMapIdx == 2.0f)
	{
		TextureColor = Texture2.Sample(Sampler2, input.uv);
	}
	else if (input.tileMapIdx == 3.0f)
	{
		TextureColor = Texture3.Sample(Sampler3, input.uv);
	}
    
    
    //�о�� �׸������� �ȼ������� ���ǹ����� ��
    [flatten]
	if (TextureColor.r == 1.0f &&
        TextureColor.g == 0.0f &&
        TextureColor.b == 1.0f)
	{
        //�ش� �ȼ��� ��������ʴ´�.
		discard;
	}
    
	TextureColor = TextureColor + (input.color * 2.0f - 1.0f);
    
	TextureColor = saturate(TextureColor);

    
    
	float2 Minus = input.position.xy - ScreenPos;
	float dis = Minus.x * Minus.x + Minus.y * Minus.y;
	dis = sqrt(dis);
	if (Select == 0.0f)
	{
		if (dis > Radius)
		{
			TextureColor.rgb += (OutColor.rgb * 2.0f - 1.0f);
		}
		else
		{
			TextureColor.rgb += (LightColor.rgb * 2.0f - 1.0f);
		}
	}
	else
	{
		float temp = 1.0f - saturate(dis / Radius);
		TextureColor.rgb += (LightColor.rgb * 2.0f - 1.0f);
		TextureColor.rgb *= temp;
	}
	TextureColor = saturate(TextureColor);
    
	if (input.tileState == 1.0f)
	{
		return TextureColor + float4(0.3, 0, 0, 0);
	}
    
	return TextureColor;
}