float4 camPos;
sampler samp;
textureCUBE envMap;
texture3D perlin;
float time;

texture2D screenColor;
texture2D screenDepth;
matrix viewProjMtx;
float2 viewportDim;
float nearZ;
float maxDistance = 30.0f;


float4 screenSpaceRayCast(float3 wOrg, float3 wDir)
{
    //worldspace ray end
    float3 wEnd = wOrg + wDir * maxDistance;
    //NDC ray endpoints
    float4 ssOrg = mul(viewProjMtx, float4(wOrg, 1.0f));
    float4 ssEnd = mul(viewProjMtx, float4(wEnd, 1.0f));

    if (ssEnd.w < nearZ) {
        float toNearZ = maxDistance * (nearZ - ssOrg.w) / (ssEnd.w - ssOrg.w);
        wEnd = wOrg + wDir * toNearZ;
        ssEnd = mul(viewProjMtx, float4(wEnd, 1.0f));
    }

    ssOrg = float4(ssOrg.xyz, 1.0f) / ssOrg.w;
    ssEnd = float4(ssEnd.xyz, 1.0f) / ssEnd.w;
    ssOrg.xy = (float2(ssOrg.x, -ssOrg.y) + 1.0f) * viewportDim / 2.0f;
    ssEnd.xy = (float2(ssEnd.x, -ssEnd.y) + 1.0f) * viewportDim / 2.0f;

    float2 delta = (ssEnd.xy - ssOrg.xy);
    bool coordSwap = false;
    if (abs(delta.x) < abs(delta.y))
    {
        //DDA will iterate vertically, swap coordinates
        delta = delta.yx;
        ssOrg.xy = ssOrg.yx;
        ssEnd.xy = ssEnd.yx;
        coordSwap = true;
    }

    //Iteration direction:
    float stepDir = sign(delta.x);
    float4 dP = stepDir * (ssEnd - ssOrg) / delta.x;
    float4 P = ssOrg;
    float endX = stepDir * ssEnd.x;
    for (; (P.x * stepDir) < endX; P += dP)
    {
        int3 pxCoord = int3(coordSwap ? P.yx : P.xy, 0);
        float screenZ = screenDepth.Load(pxCoord).r;
        float rayZ = P.z + 0.5f * dP.z;
        if (screenZ < rayZ)
            return screenColor.Load(pxCoord);

    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}


struct PSInput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION0;
    float3 worldPos : POSITION1;
};

float3 intersectRay(float3 p, float3 d)
{
    float tx = max((1 - p.x) / d.x, (-1 - p.x) / d.x);
    float ty = max((1 - p.y) / d.y, (-1 - p.y) / d.y);
    float tz = max((1 - p.z) / d.z, (-1 - p.z) / d.z);

    float t = min(tx, min(ty, tz));
    return p + d * t;
}

float fresnel(float n1, float n2, float3 N, float3 V)
{
    float f0 = (n2 - n1) / (n1 + n2);
    f0 *= f0;
    if (dot(N, V) < 0) N = -N;
    float costh = max(dot(N, V), 0);
    return f0 + (1.0f - f0) * pow((1.0f - costh), 5);
}

float4 main(PSInput i) : SV_TARGET
{
    float3 viewVec = normalize(camPos.xyz - i.worldPos);
    float3 tex = float3(i.localPos.xz * 10.0f, time);
    float ex = perlin.Sample(samp, tex);
    tex.x += 0.5f;
    tex.y += 0.5f;
    tex.z += 0.5f;
    float ez = perlin.Sample(samp, tex);

    float n1 = 1.0f;
    float n2 = 4.0f / 3.0f;

    float3 norm = normalize(float3(ex, 20.0f, ez));
    float3 refl = reflect(-viewVec, norm);
    float refrCoeff = n1 / n2;
    if (dot(norm, viewVec) < 0) { refrCoeff = n2 / n1; }
    float3 refr = refract(-viewVec, norm, refrCoeff);

    float fres = fresnel(n1, n2, norm, viewVec);
    //fres = 1.0f - fres;
    float3 color;


    float3 tRefl = intersectRay(i.localPos, refl);
    float3 colorRefl = envMap.Sample(samp, tRefl).rgb;
    float3 tRefr = intersectRay(i.localPos, refr);
    float3 colorRefr = envMap.Sample(samp, tRefr).rgb;
    float4 ssReflColor =  screenSpaceRayCast(i.worldPos, refl);
    colorRefl = lerp(colorRefl,   ssReflColor.rgb, ssReflColor.a);
    float4 ssRefrColor = screenSpaceRayCast(i.worldPos, refr);
    colorRefr = lerp(colorRefr, ssRefrColor.rgb, ssRefrColor.a);
    return ssReflColor;
    if (any(refr))
    {
        color = fres * colorRefl + (1.0f - fres) * colorRefr;
        if (dot(norm, viewVec) < 0)
        {
            tRefr.y = -tRefr.y;
            color = envMap.Sample(samp, tRefr).rgb;
        }
    }
    else
    {
        color = colorRefl;
    }

    color = pow(color, 0.4545f);
    return float4(color,1.0f);
}
