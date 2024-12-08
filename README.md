# Attribute Type Qualifiers

- `uniform`: constant accross the face
- `smooth`: perspective-correct interpolation
- `noperspective`: linear interpolation (Barycentric coordinates)

**Unimplemented**: flat ("provoking" vertex), closest (would-be-nice)

# Concise Description of Coordinate Systems

## World Coordinates

"Bake" all modeling transformations. Simple.

## Camera Coordinates

Change of coordinates so that -w is camera gaze. Simple

## Normalized Device Coordinates

Map the space inside camera frustum to $ [-1,1] ^ 3 $

### Notes
- Z-values here will get mapped into Z-buffer element range $ [0,1] $.
- **IS** used for clipping.
- **IS NOT** the input to rasterizer, technically.

## Viewport Coordinates

Transform all point in Canonical Viewing Volume to an image plane of size `image_width * image_height`
that is actually *slightly shifted* towards $ (-0.5,-0.5) $ so that 
**integer coordinates are the center of a pixel sample**.

This coordinate system is the first quadrant of the canonical 2D coordinate system (i.e. $Y$-up).

Original $Z$ values are not touched! It is still in range $[-1,1]$, unlike $X$ and $Y$.

## Notes
- **IS NOT** used for clipping. The code for clipping should reside in Renderer!
- Viewport Coordinates are floating-point!
- Sooner or later coordinates need to be "dehomogenized"
- Points here are fed to rasterized to obtain integer-aligned *fragments* that have a different coordinate vectors

## Pixel Coordinates

Are different from Viewport Coordinates! Are non-negative integers! Have $Y$-down! Only used for outputting!

# Weird Coordinate Naming

| Variable Name | Type                               | What's This?                                            | Range     |
|---------------|------------------------------------|---------------------------------------------------------|-----------|
| wc            | Vec3f                              | World Coordinates                                       | Arbitrary |
| cc            | Vec4f                              | Camera Coordinates                                      | Arbitrary |
| ndc           | Vec4f                              | Canonical Viewing Volume/Normalized Device Coordinates  | $[-1,1]^3$ |
| vpc           | Vec4f                              | Viewport/Image Plane Coordinates                        | See Above      |
| pc            | PixelCoordinates (natural numbers) | Final Pixel Buffer Coordinates (first row, then column) | ViewConfig-dependent          |

# Exhaustive "Product Story"

## Step 0: World (input)

### Input

A scene in some format.

### Processing

"Bake" the scene.

### Output
- Every object is composed of `Face`, though objects themselves are not exposed to outside, so a `std::vector<Face>` may as well be the input world object. Every `Face` contains `Vertex`. Both have an associated *attribute* object.


     + *Attributes*: Vertices and Faces may have attributes that need to be passed down to its resultant Fragments. Vertices have `ceng477_color`. Faces may have an associated material.

## Step 1: Camera (`t_camera`)

### Input

- A single `Face`

### Processing

- Calculate camera coordinates of vertices. Simple

### Output

- A `S1Face`

## Step 2: Back-face culling

### Input

- A single `S1Face`

### Processing

- Calculate homogenous normal of `Face` in Camera space.
- Calculate dot product of this normal and camera gaze. Try culling.

### Output

- A single `S1Face` or nothing

## Step 3: Device (`t_projection`)

### Input

- A single `S1Face`

### Processing

- 3 matrix multiplications. Simple.

### Output

- A single `S3Face`

## Step 4: Clipping

### Input

- A single `S3Face`

### Processing

- Clip against the NDC planes by performing Sutherland-Hodgman algorithm

### Output

- A single `S4Polygon`

## Step 5: Rasterizer (Line Drawing, Solid Drawing, Attribute Interpolation)

### Input

- A single `S4Polygon`

### Processing

- First, take polygon vertices to viewport coordinates

### Output

- A `Fragment`, that has integer Pixel Coordinates in **conventional 2D graphics directions**
where $Y$ becomes larger as we go *down*, unlike Viewport Coordinates!

## Step 6: Fragment Processing (Depth Test)

### Input

- *All* of the resultant many a `Fragment`.

### Processing

- Initialize Z-buffer and perform Depth Test by iterating through all Fragments

### Output

- *Some* of the resultant many a `Fragment`.



## Step 7: Write to Image

*Self-explanatory*

---

# TODO

- Tweak Rasterizer interface
- Midpoint Algorithm
- Filling in shapes
