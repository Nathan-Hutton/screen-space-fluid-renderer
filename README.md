# Real-Time Fluid Rendering

## Video

[<img src="https://img.youtube.com/vi/rzK7BJzq90k/maxresdefault.jpg" width="900"
/>](https://www.youtube.com/embed/rzK7BJzq90k)

## Description

An OpenGL/GLSL implementation of real-time rendering of cached fluid simulations. This is acheived in screen-space by generating a depth-map of the particle data and applying a narrow-range filter to refine the desired surface shape. The final render of the fluid surface includes realistic shading such as reflections, refraction, and shadows all at interactive speed.

Optionally, caustics lighting can also be rendered, which is calculated using a modified caustics mapping technique.

Papers and other resources used to create this implementation can be found in the references section.

## References

- [A Narrow-Range Filter for Screen-Space Fluid Rendering](https://ttnghia.github.io/posts/narrow-range-filter/)
- [Interactive Screen-Space Surface Rendering of Dynamic Particle Clouds](https://doi.org/10.1080/2151237X.2009.10129282)
- [Caustics Mapping: An Image-space Technique for Real-time Caustics](http://www.klayge.org/material/3_12/Caustics/caustics.pdf)
- [cyCodeBase by Cem Yuksel](https://www.cemyuksel.com/cyCodeBase/code.html)
