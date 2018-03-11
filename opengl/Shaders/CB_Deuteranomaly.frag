uniform sampler2D texture;

const mat4 mDeuteranomaly = mat4( 0.8  ,  0.2  ,  0.0  ,  0.0,
								 0.258,  0.742,  0.0  ,  0.0,
								 0.0  ,  0.142,  0.858,  0.0,
                                 0.0  ,  0.0  ,  0.0  ,  1.0 );

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	gl_FragColor = pixel*mDeuteranomaly;
}
