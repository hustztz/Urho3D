#ifdef COMPILEPS
#line 50000

vec2 hash22( vec2 p )
{
	p = vec2( dot(p,vec2(127.1,311.7)),
			  dot(p,vec2(269.5,183.3)) );

	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float simplex_noise(vec2 p)
{
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;
	
	// 变换到新网格的(0, 0)点
    vec2 i = floor(p + (p.x + p.y) * K1);

	// i - (i.x+i.y)*K2换算到旧网格点
    // a:变形前输入点p到该网格点的距离
    vec2 a = p - (i - (i.x + i.y) * K2);
    vec2 o = (a.x < a.y) ? vec2(0.0, 1.0) : vec2(1.0, 0.0);

    // 新网格(1.0, 0.0)或(0.0, 1.0)
    // b = p - (i+o - (i.x + i.y + 1.0)*K2);
    vec2 b = a - o + K2;

    // 新网格(1.0, 1.0)
    // c = p - (i+vec2(1.0, 1.0) - (i.x+1.0 + i.y+1.0)*K2);
    vec2 c = a - 1.0 + 2.0 * K2;

	// 计算每个顶点的权重向量，r^2 = 0.5
    vec3 h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);

    // 每个顶点的梯度向量和距离向量的点乘，然后再乘上权重向量
    vec3 n = h * h * h * h * vec3(dot(a, hash22(i)), dot(b, hash22(i + o)), dot(c, hash22(i + 1.0)));

    // 之所以乘上70，是在计算了n每个分量的和的最大值以后得出的，这样才能保证将n各个分量相加以后的结果在[-1, 1]之间
    return dot(vec3(70.0, 70.0, 70.0), n);
}
#endif