import math
import random
import struct

from pynodegl import Texture, Shader, TexturedShape, Rotate, AnimKeyFrameScalar, Triangle
from pynodegl import Quad, UniformVec4, Camera, Group
from pynodegl import Media, Translate, AnimKeyFrameVec3
from pynodegl import UniformScalar
from pynodegl import Compute, ComputeShader, Shape2, BufferFloat, BufferVec2, BufferVec3, BufferVec4

from pynodegl_utils.misc import scene

from OpenGL import GL

@scene({'name': 'size', 'type': 'range', 'range': [0,1.5], 'unit_base': 1000})
def triangle(cfg, size=0.5):
    frag_data = '''
#version 100
precision mediump float;
varying vec2 var_tex0_coords;
void main(void)
{
    vec2 c = var_tex0_coords;
    gl_FragColor = vec4(c.y-c.x, c.x, 1.0-c.y, 1.0);
}'''

    b = size * math.sqrt(3) / 2.0
    c = size * 1/2.

    triangle = Triangle((-b, -c, 0), (b, -c, 0), (0, size, 0))
    s = Shader(fragment_data=frag_data)
    node = TexturedShape(triangle, s, Texture())
    node = Rotate(node, axis=(0,0,1))
    node.add_animkf(AnimKeyFrameScalar(0, 0),
                    AnimKeyFrameScalar(cfg.duration, -360*2))
    return node

@scene({'name': 'n', 'type': 'range', 'range': [2,10]})
def fibo(cfg, n=8):
    frag_data = '''
#version 100
precision mediump float;
uniform vec4 color;
void main(void) {
    gl_FragColor = color;
}'''

    cfg.duration = 5

    s = Shader(fragment_data=frag_data)

    fib = [0, 1, 1]
    for i in range(2, n):
        fib.append(fib[i] + fib[i-1])
    fib = fib[::-1]

    shift = 1/3. # XXX: what's the exact math here?
    shape_scale = 1. / ((2.-shift) * sum(fib))

    orig = (-shift, -shift, 0)
    g = None
    root = None
    for i, x in enumerate(fib[:-1]):
        w = x * shape_scale
        gray = 1. - i/float(n)
        color = [gray, gray, gray, 1]
        q = Quad(orig, (w, 0, 0), (0, w, 0))
        tshape = TexturedShape(q, s)
        tshape.add_uniforms(UniformVec4("color", value=color))

        new_g = Group()
        rot = Rotate(new_g, axis=(0,0,1), anchor=orig)
        rot.add_animkf(AnimKeyFrameScalar(0, 90, "exp_in_out"),
                       AnimKeyFrameScalar(cfg.duration/2, -90, "exp_in_out"),
                       AnimKeyFrameScalar(cfg.duration, 90))
        if g:
            g.add_children(rot)
        else:
            root = rot
        g = new_g
        new_g.add_children(tshape)
        orig = (orig[0] + w, orig[1] + w, 0)

    root = Camera(root)

    root.set_eye(0.0, 0.0, 2.0)
    root.set_up(0.0, 1.0, 0.0)
    root.set_perspective(45.0, cfg.aspect_ratio, 1.0, 10.0)
    return root

@scene({'name': 'dim', 'type': 'range', 'range': [1,50]})
def cropboard(cfg, dim=15):
    cfg.duration = 10

    kw = kh = 1. / dim
    qw = qh = 2. / dim
    tqs = []

    s = Shader()
    m = Media(cfg.medias[0].filename)
    t = Texture(data_src=m)

    for y in range(dim):
        for x in range(dim):
            corner = (-1. + x*qw, 1. - (y+1.)*qh, 0)
            q = Quad(corner, (qw, 0, 0), (0, qh, 0))

            q.set_uv_corner(x*kw, 1. - (y+1.)*kh)
            q.set_uv_width(kw, 0)
            q.set_uv_height(0, kh)

            tshape = TexturedShape(q, s, t)

            startx = random.uniform(-2, 2)
            starty = random.uniform(-2, 2)
            trn = Translate(tshape)
            trn.add_animkf(AnimKeyFrameVec3(0, (startx, starty, 0), "exp_out"),
                           AnimKeyFrameVec3(cfg.duration*2/3., (0, 0, 0)))
            tqs.append(trn)

    return Group(children=tqs)

@scene({'name': 'freq_precision', 'type': 'range', 'range': [1,10]},
       {'name': 'overlay', 'type': 'range', 'unit_base': 100})
def audiotex(cfg, freq_precision=7, overlay=0.6):
    media = cfg.medias[0]
    cfg.duration = media.duration

    freq_line = 2                           # skip the 2 audio channels
    freq_line += (10 - freq_precision) * 2  # 2x10 lines of FFT

    fft1, fft2 = freq_line + 0.5, freq_line + 1 + 0.5

    frag_data = '''
#version 100
precision mediump float;

uniform sampler2D tex0_sampler;
uniform sampler2D tex1_sampler;
varying vec2 var_tex0_coords;

void main()
{
    vec4 audio_pix;
    vec4 video_pix = texture2D(tex1_sampler, var_tex0_coords);
    vec2 sample_id_ch_1 = vec2(var_tex0_coords.x,      0.5 / 22.);
    vec2 sample_id_ch_2 = vec2(var_tex0_coords.x,      1.5 / 22.);
    vec2  power_id_ch_1 = vec2(var_tex0_coords.x, %(fft1)f / 22.);
    vec2  power_id_ch_2 = vec2(var_tex0_coords.x, %(fft2)f / 22.);
    float sample_ch_1 = texture2D(tex0_sampler, sample_id_ch_1).x;
    float sample_ch_2 = texture2D(tex0_sampler, sample_id_ch_2).x;
    float  power_ch_1 = texture2D(tex0_sampler,  power_id_ch_1).x;
    float  power_ch_2 = texture2D(tex0_sampler,  power_id_ch_2).x;
    power_ch_1 = sqrt(power_ch_1);
    power_ch_2 = sqrt(power_ch_2);
    sample_ch_1 = (sample_ch_1 + 1.) / 8.; // [-1;1] -> [0    ; 0.25]
    sample_ch_2 = (sample_ch_2 + 3.) / 8.; // [-1;1] -> [0.25 ; 0.5 ]
    power_ch_1 = clamp(power_ch_1, 0., 1.) / 4.; // [0 ; +oo] -> [0 ; 0.25]
    power_ch_2 = clamp(power_ch_2, 0., 1.) / 4.; // [0 ; +oo] -> [0 ; 0.25]

    float diff_wave_ch_1 = abs(sample_ch_1 - var_tex0_coords.y);
    float diff_wave_ch_2 = abs(sample_ch_2 - var_tex0_coords.y);
    if (diff_wave_ch_1 < 0.003) {
        audio_pix = vec4(0.5, 1.0, 0.0, 1.0);
    } else if (diff_wave_ch_2 < 0.003) {
        audio_pix = vec4(0.0, 1.0, 0.5, 1.0);
    } else if (var_tex0_coords.y > 0.75 - power_ch_1 && var_tex0_coords.y < 0.75) {
        audio_pix = vec4(1.0, 0.5, 0.0, 1.0);
    } else if (var_tex0_coords.y > 1.   - power_ch_2 && var_tex0_coords.y < 1.) {
        audio_pix = vec4(1.0, 0.0, 0.5, 1.0);
    } else {
        audio_pix = vec4(0.0, 0.0, 0.0, 1.0);
    }
    gl_FragColor = mix(video_pix, audio_pix, %(overlay)f);
}
''' % {'fft1': fft1, 'fft2': fft2, 'overlay': overlay}

    q = Quad((-1, -1, 0), (2, 0, 0), (0, 2, 0))

    audio_m = Media(media.filename, audio_tex=1)
    audio_tex = Texture(data_src=audio_m)

    video_m = Media(media.filename)
    video_tex = Texture(data_src=video_m)

    s = Shader(fragment_data=frag_data)
    tshape = TexturedShape(q, s, [audio_tex, video_tex])
    return tshape

@scene()
def compute_shader(cfg):
    compute_data = '''
#version 430

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img0;

void main(void)
{
    vec4 pixel = vec4(1.0, 0.0, 0.0, 1.0);
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    imageStore(img0, pixel_coords, pixel);
}'''
    t = Texture(width=512, height=512, format=GL.GL_RGBA, internal_format=GL.GL_RGBA32F, type=GL.GL_FLOAT)
    cs = ComputeShader(compute_data)
    c = Compute(512, 512, 1, cs)
    c.add_textures(t)

    g = Group()

    q = Quad((-1, -1, 0), (2, 0, 0), (0, 2, 0))
    m = Media(cfg.medias[0].filename, initial_seek=5)
    s = Shader()
    tshape = TexturedShape(q, s, t)
    g.add_children(c, tshape)
    return g

@scene()
def compute_shader_with_buffers(cfg):
    compute_data = '''
#version 430

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Position {
    float x, y, z;
};

layout (std430, binding = 0) buffer positions_buffer {
    Position positions[];
};

struct Velocity {
    float x, y;
};

layout (std430, binding = 1) buffer velocities_buffer {
    Velocity velocities[];
};

layout (std430, binding = 2) buffer lifes_buffer {
    float lifes[];
};

uniform float time;

void main(void)
{
    uint i = gl_GlobalInvocationID.x +
             gl_GlobalInvocationID.y * 1024;

    Position position = positions[i];
    Velocity velocity = velocities[i];
    float life = lifes[i];
    float gravity = 0.001;
    float attenuation = 0.8;

    /* update position */
    if (life <= 0.0) {
        life = 1.0;
        position.y = 1.0;
    }

    life -= 0.001;
    position.x += velocity.x;
    position.y += velocity.y;

    /* update velocity */
    velocity.y -= gravity;

    if (position.x <= -1.0) {
        position.x = -1.0;
        vec2 v1 = vec2(velocity.x, velocity.y);
        vec2 v2 = vec2(-1.0, velocity.y);
        v1 = reflect(v1, v2) * attenuation;
        velocity = Velocity(v1.x, v1.y);
    } else if (position.x >= 1.0) {
        position.x = 1.0;
        vec2 v1 = vec2(velocity.x, velocity.y);
        vec2 v2 = vec2(1.0, velocity.y);
        v1 = reflect(v1, v2) * attenuation;
        velocity = Velocity(v1.x, v1.y);
    } else if (position.y <= -1.0) {
        position.y = -1.0;
        vec2 v1 = vec2(velocity.x, velocity.y);
        vec2 v2 = vec2(velocity.x, -1.0);
        v1 = reflect(v1, v2) * attenuation;
        velocity = Velocity(v1.x, v1.y);
    }

    positions[i] = position;
    velocities[i] = velocity;
    lifes[i] = life;
}
'''

    vertex_data = '''
#version 100

attribute vec4 ngl_position;
attribute vec2 velocities_buffer;
attribute float lifes_buffer;

uniform mat4 ngl_modelview_matrix;
uniform mat4 ngl_projection_matrix;
uniform mat3 ngl_normal_matrix;

varying vec2 velocity;
varying float life;

void main()
{
    gl_Position = ngl_projection_matrix * ngl_modelview_matrix * ngl_position;
    velocity = velocities_buffer;
    life = lifes_buffer;
}
'''

    fragment_data = '''
#version 100

precision highp float;

varying vec2 velocity;
varying float life;

float force(vec2 v)
{
    return sqrt(v.x * v.x + v.y * v.y) * 50.0;
}

void main(void)
{
    vec4 color;
    color = vec4(0.0, 0.6, 0.8, 1.0);
    color.r = force(velocity);
    color.g = color.g * life;
    color.b = color.b * life;

    gl_FragColor = color;
}'''

    cfg.duration = 5

    g = Group()

    n = 1024
    m = 1024
    o = 1
    p = n * m * o

    positions_data = b''
    velocities_data = b''
    lifes_data = b''

    for i in range(p):
        positions_data += struct.pack(
            'fff',
            random.uniform(-1.0, 1.0),
            random.uniform(0.0, 1.0),
            0.0
        )

        velocities_data += struct.pack(
            'ff',
            random.uniform(-0.01, 0.01),
            random.uniform(-0.05, 0.05),
        )

        lifes_data += struct.pack(
            'f',
            random.uniform(0.0, 1.0),
        )

    positions = BufferVec3("positions_buffer", p)
    positions.set_data(positions_data)

    velocities = BufferVec2("velocities_buffer", p)
    velocities.set_data(velocities_data)

    lifes = BufferFloat("lifes_buffer", p)
    lifes.set_data(lifes_data)

    time = UniformScalar("time", value=0.0)
    time.add_animkf(AnimKeyFrameScalar(0.0, 0.0),
                    AnimKeyFrameScalar(cfg.duration, 1.0))

    cs = ComputeShader(compute_data)

    c = Compute(n, m, o, cs)
    c.add_buffers(positions, velocities, lifes)
    c.add_uniforms(time)

    q = Shape2(positions)
    q.set_draw_type(GL.GL_UNSIGNED_INT)
    q.set_draw_mode(GL.GL_POINTS)

    m = Media(cfg.medias[0].filename, initial_seek=5)
    s = Shader(vertex_data=vertex_data, fragment_data=fragment_data)

    tshape = TexturedShape(q, s)
    tshape.add_attributes(velocities)
    tshape.add_attributes(lifes)

    g.add_children(c, tshape)

    return Camera(g)
