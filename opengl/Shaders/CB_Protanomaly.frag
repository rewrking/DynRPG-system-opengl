uniform sampler2D texture;

const mat4 mProtanomaly = mat4( 0.817, 0.183	, 0.0  , 0.0,
							   0.333, 0.667	, 0.0  , 0.0,
							   0.0  , 0.125	, 0.875, 0.0,
							   0.0  ,  0.0  , 0.0  , 1.0 );

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	gl_FragColor = pixel*mProtanomaly;
}
