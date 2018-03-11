uniform sampler2D texture;

const mat4 mTritanomaly = mat4( 0.967,  0.033,  0.0  ,  0.0,
							    0.0  ,  0.733,  0.267,  0.0,
								0.0  ,  0.183,  0.817,  0.0,
                                0.0  ,  0.0  ,  0.0  ,  1.0 );

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	gl_FragColor = pixel*mTritanomaly;
}
