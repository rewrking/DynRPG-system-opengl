uniform sampler2D texture;

const mat4 mAchromatomaly = mat4(0.618,  0.320,  0.062,  0.0,
								 0.163,  0.775,  0.062,  0.0,
								 0.163,  0.320,  0.516,  0.0,
                                 0.0  ,  0.0  ,  0.0  ,  1.0 );

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	gl_FragColor = pixel*mAchromatomaly;
}
