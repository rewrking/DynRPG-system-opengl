/* Scale2x (aka AdvMame2x)
	* Algorithm found at http://scale2x.sourceforge.net/algorithm.html
	* Adapted for use by shaders
*/

uniform sampler2D texture;
uniform vec2 texture_dimensions;

void main() {
	// o = offset, the width of a pixel
	vec2 o = 1.0 / texture_dimensions;
	vec2 coord = gl_TexCoord[0].xy;
	// texel arrangement
	// A B C
	// D E F
	// G H I
	vec4 A = texture2D(texture, coord + vec2( -o.x,  o.y));
	vec4 B = texture2D(texture, coord + vec2(    0,  o.y));
	vec4 C = texture2D(texture, coord + vec2(  o.x,  o.y));
	vec4 D = texture2D(texture, coord + vec2( -o.x,    0));
	vec4 E = texture2D(texture, coord + vec2(    0,    0));
	vec4 F = texture2D(texture, coord + vec2(  o.x,    0));
	vec4 G = texture2D(texture, coord + vec2( -o.x, -o.y));
	vec4 H = texture2D(texture, coord + vec2(    0, -o.y));
	vec4 I = texture2D(texture, coord + vec2( o.x, -o.y));
	vec2 p = coord * texture_dimensions;
	// p = the position within a pixel [0...1]
	p = p - floor(p);
	if (p.x > .5) {
		if (p.y > .5) {
			// Top Right
			gl_FragColor = B == F && B != D && F != H ? F : E;
		} else {
			// Bottom Right
			gl_FragColor = H == F && D != H && B != F ? F : E;
		}
	} else {
		if (p.y > .5) {
			// Top Left
			gl_FragColor = D == B && B != F && D != H ? D : E;
		} else {
			// Bottom Left
			gl_FragColor = D == H && D != B && H != F ? D : E;
		}
	}
}