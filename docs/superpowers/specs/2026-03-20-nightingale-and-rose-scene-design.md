# Nightingale And Rose Scene Design

## Goal

Replace the current cube viewer with a more artistic mobile-friendly scene inspired by "The Nightingale and the Rose" while preserving the existing BLE orientation data pipeline.

## Scope

Included in this round:

- keep the current `roll/pitch` BLE transport unchanged
- replace the cube with a stylized 3D-like stage
- render a moonlit rose as the main subject
- add a perched nightingale silhouette near the rose
- keep the telemetry and connection controls
- let the full sculpture rotate with live orientation data

Explicitly excluded:

- firmware protocol changes
- yaw support
- true 3D rendering engine
- skeletal bird animation
- multi-scene transitions

## Scene Direction

Chosen direction: **Moonlit sculpture**

The scene should feel like a decorative object on a small luminous stage rather than a flat illustration. The design emphasizes volume, glow, moonlight, and layered depth.

## Structure

The web scene keeps the current app shell but swaps the cube for a composed stage:

- night-sky backdrop with moon halo
- floating sculpture rig that rotates with `roll/pitch`
- layered rose bloom made from overlapping petals
- stem and leaves to anchor the composition
- perched nightingale silhouette near the bloom
- glowing pedestal and soft shadow beneath

## Motion Mapping

The existing BLE values remain the input:

- `pitch` drives X-axis tilt
- `roll` drives Y-axis rotation

The entire sculpture rig rotates as one object so the presentation remains legible and stable. Small built-in base rotations may be added to preserve a pleasant viewing angle.

## Visual Language

- deep midnight gradient background
- pale moonlight and soft fog glow
- crimson-to-rose petal gradients
- warm highlight edges on petals
- dark bird silhouette with a faint edge light
- pedestal glow to reinforce depth

## Implementation Constraints

- use HTML/CSS only, no Three.js
- keep the DOM understandable and maintainable
- preserve mobile performance
- keep BLE controls and telemetry readable

## Verification

Manual verification is sufficient for this round:

- page loads on phone
- BLE still connects
- telemetry still updates
- sculpture visibly rotates with board motion
- the scene reads as a dimensional rose-and-bird composition rather than a flat card
