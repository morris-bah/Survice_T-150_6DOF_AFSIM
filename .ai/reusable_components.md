# Reusable Components

> Custom AFSIM components built in this project that may be useful elsewhere.
> Items marked [PROMOTE] should be added to the shared component registry.
>
> Agents: When you create a new custom component (mover, sensor, processor, etc.),
> register it here with its type name, description, and key configuration parameters.

---

## WSF_ROTORCRAFT_6DOF_MOVER
- **Plugin**: `plugins/rotorcraft_6dof/`
- **Base class**: `WsfRouteMover`
- **Description**: Physics-based 6-DOF rotorcraft mover with Catmull-Rom spline navigation
- **Key parameters**: rotor configuration (COAXIAL_OCTOCOPTER, COAXIAL_QUAD), motor count, performance limits

## T150_BATTERY_PROCESSOR
- **Plugin**: `plugins/t150_battery/`
- **Base class**: `WsfProcessor`
- **Description**: Real-time Li-Ion battery state-of-charge, voltage sag, and discharge curves. Reads power draw from `PROP_TOTAL_POWER` AuxData key.
- **Key parameters**: `battery_capacity_wh`, `nominal_voltage`, cell chemistry config
- **Exports AuxData**: `BATTERY_SOC` (0-1), `BATTERY_VOLTAGE` (V under load)

## T150_PROPULSION_PROCESSOR
- **Plugin**: `plugins/t150_propulsion/`
- **Base class**: `WsfProcessor`
- **Description**: Electric motor model with thermal effects, ground effect, payload dynamics. Dynamic power based on flight state (velocity, acceleration, bank angle).
- **Exports AuxData**: `PROP_TOTAL_POWER` (W)

## T150_VRS_PROCESSOR
- **Plugin**: `plugins/t150_vrs/`
- **Base class**: `WsfProcessor`
- **Description**: Vortex Ring State detection and warning system for rotorcraft.
- **Exports AuxData**: `VRS_MARGIN` (0 = in VRS, 1 = safe)

## T150_SLUNG_LOAD_PROCESSOR
- **Plugin**: `plugins/t150_slung_load/`
- **Base class**: `WsfProcessor`
- **Description**: Cable-suspended payload dynamics — pendulum motion, cable tension, attachment dynamics.

## T150_WEAPON_PROCESSOR
- **Plugin**: `plugins/t150_weapon/`
- **Base class**: `WsfProcessor`
- **Description**: Multi-weapon support, accurate ballistics, weapon recoil effects on 6DOF platform via external force integration.
