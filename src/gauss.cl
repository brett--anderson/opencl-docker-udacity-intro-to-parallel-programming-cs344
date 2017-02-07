__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void process(
        __read_only image2d_t src,
        __constant float * mask,
        __write_only image2d_t dst,
        __private int size,
        __private int W,
        __private int H
    ) {

    const int2 pos = {get_global_id(0), get_global_id(1)};

    // VARIABLES
    unsigned int x, y;
    float4 value;
    float4 color;
    int i, xOff, yOff, center;


    // CALCULATE VARIABLES
    center = size / 2;

    // PROCESS THE PIXEL
    if((pos.x > center && pos.x < (W-center)) &&
     (pos.y > center && pos.y < (H-center)))
    {
        value=(float4)(0,0,0,0);
        for(y = 0; y < size; y++)
        {
            yOff = (y-center);
            for(x = 0; x < size; x++)
            {
                xOff = (x-center);

                color = read_imagef(src, sampler, (int2)(pos.x + xOff, pos.y + yOff));
                value.x += mask[y * size + x] * color.x;
                value.y += mask[y * size + x] * color.y;
                value.z += mask[y * size + x] * color.z;

                
                        
            }
        }
        write_imagef(dst, pos, value);
        
    }
    else // VALUE REMAINS IF WE ARE NEAR THE EDGE
    {
        color = read_imagef(src, sampler, pos);
        write_imagef(dst, pos, color);
    }

}