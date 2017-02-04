const sampler_t sampler =   CLK_NORMALIZED_COORDS_FALSE |
                            CLK_ADDRESS_CLAMP_TO_EDGE |
                            CLK_FILTER_NEAREST;

__kernel void process(__read_only  image2d_t src,
                        __write_only image2d_t dst)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    float4 color;

    color = read_imagef(src, sampler, (int2)(x, y));
    write_imagef(dst, (int2)(x,y), (float4)(color.x, color.y, color.z, 0));
}