#version 430 core

layout( triangles ) in;
layout( triangle_strip, max_vertices = 7 ) out;

uniform float pctExtend;
uniform float edgeWidth;

in VS_GS{
	in vec3 VNormal;
	in vec3 VPosition;
	// ndotv도 triangle마다 넘어와야 interpolation함
	in float ndv;
} gs_in[];

out GS_FS{
	out vec3 GNormal;
	out vec3 GPosition;
} gs_out;

flat out int GIsEdge;

void emitEdgeQuad(vec3 a, vec3 b){

	GIsEdge = 1;
	vec2 ext	= pctExtend * ( b.xy - a.xy );
	vec2 v		= normalize( b.xy - a.xy);
	vec2 n		= vec2(-v.y, v.x) * edgeWidth;

	gl_Position = vec4(a.xy - ext + n, a.z, 1.0); EmitVertex();
	gl_Position = vec4(a.xy - ext - n, a.z, 1.0); EmitVertex();
	gl_Position = vec4(b.xy + ext + n, b.z, 1.0); EmitVertex();
	gl_Position - vec4(b.xy + ext - n, b.z, 1.0); EmitVertex();

	EndPrimitive();
}

void findEdge(){
	
	
	vec3 e0, e1;

	vec3 p0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
	vec3 p1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
	vec3 p2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;

	float nv0 = gs_in[0].ndv;
	float nv1 = gs_in[1].ndv;
	float nv2 = gs_in[2].ndv;

	bool b0 = nv0 <= 0;
	bool b1 = nv1 <= 0;
	bool b2 = nv2 <= 0;

	if(b0!=b1 && b1==b2){
		e0 = mix(p0, p1, nv0/(nv0-nv1));
		e1 = mix(p0, p2, nv0/(nv0-nv2));
		emitEdgeQuad(e0, e1);
	}
	else if (b0!=b1&&b0==b2){
		e0 = mix(p1, p0, nv1/(nv1-nv0));
		e1 = mix(p1, p2, nv1/(nv1-nv2));
		emitEdgeQuad(e0, e1);
	}
	else if (b0!=b2&&b0==b1){
		e0 = mix(p2, p0, nv2/(nv2-nv0));
		e1 = mix(p2, p1, nv2/(nv2-nv1));
		emitEdgeQuad(e0, e1);
	}

}

void main(){
	
	findEdge();
	
	// non-edge triangles
	GIsEdge = 0;
	gl_Position = gl_in[0].gl_Position;
	gs_out.GPosition = gs_in[0].VPosition;
	gs_out.GNormal = gs_in[0].VNormal;
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	gs_out.GPosition = gs_in[1].VPosition;
	gs_out.GNormal = gs_in[1].VNormal;
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	gs_out.GPosition = gs_in[2].VPosition;
	gs_out.GNormal = gs_in[2].VNormal;
	EmitVertex();

	EndPrimitive();
}