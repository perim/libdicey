ARPG Stats Library
==================

This is a library for doing some of the internal game state calculations for an action
role-playing game (ARPG). It can take care of tracking powers (such as health, stamina,
energy), skill cooldowns, buffs and debuffs, while implementing features such as
regeneration, temporary bonus power, and temporary and permanent upgrades.

The main C++ header defines a section dedicated to game wide setup state. You are meant
to modify these to match your game.

The library is written to be fast and there are some sacrifices made to ensure that
this is the case.

Each second you need to call the `second_tick()` function. If you want higher precision
for all or parts of the entities, also call `subsecond_tick()` a predefined number of
times per second extra. You can mark entities as `highres` to be included in this high
precision call. High precision can be important for smooth GUI elements, and you can
also decide to mark all entities shown on screen as high precision while all others are
low precision to save CPU time.

You can also mark entities as not `active` which means no further calculations will be
done on them. There is a helper function `inactive_candidate()` which will let you know
if we consider it safe to make it inactive (ie it has no timed statuses or damage over
time on it).

Powers
------

We define a 'power' as a finite resource of some kind that the entity uses
or needs and can replenish. For example health, mana, stamina, or energy. You
should set the power_types constant to the number of powers you will use.

The current and max values are communicated as percentages, but internally stored
as more fine-grained integer fractions.

Definitions:
* Regeneration - amount of power automatically regained each tick
* Recovery - any other power gain than regeneration, typically as a result of
  skills being used
* Enhanced power - extra power granted that can exceed the maximum, but once lost
  is not recovered or regenerated
* Excess power - extra power granted that can exceed the maximum as above, but is
  also constantly lost each tick
* Cost - any power reduction from skill use.

A number of different statuses (upgrades) exist for each power:
* `power_maximum` - increasing the maximum available
* `power_excess_loss_rate` - the rate at which excess power is lost
* `power_instant_damage_taken` - instant loss of this power is modified by this amount
* `power_dot_taken` - loss over time of this power is modified by this amount
* `power_recovery_modify` - modify amount gained each tick by this amount
* `power_recovery_time` - modify time a gain happens over, which means more total
  recovery in the ened
* `power_regeneration_rate` - amount of power automatically gained each tick
* `power_cost_modify` - modify the cost of using this power

You can apply a power recovery on an entity with the function
`apply_recover_effect(current_effect_second, power_type, seconds, amount)`. The
`current_effect_second` is an index into our circular buffer for recovery effects
that you can obtain from the `stats` object.

You can apply a power cost using the function `try_expense(power_type, amount)` which
returns true if we could expense the cost, and false if we could not and nothing is
changed.

Damage
------

Damage is any kind of loss of power that is not a cost. We support any number of
different damage types.

A number of different statuses exist for each:
* `damage_instant_damage` - modify the amount of instant damage taken of this type
* `damage_dot` - modify the amount of damage over time taken of this type
* `damage_dot_exponential` - modify the damage over time each second; if positive makes
  it worse, if net negative makes it less bad
* `damage_dot_time_modify` - modify the time the damage is applied to, changing total amount
* `damage_distribution_modify` - change the distribution factors of loss between powers

The order the powers are defined in determines which power loses first, and starting with
the last power. The damage distribution factor determines how much of the damage is taken
from each power until we reach the first power, which then takes on the remaining loss,
if possible. It is assumed that the first power is the most valuable, without which very
bad things will happen (eg death).

The damage distribution is how much of the damage that should be attributed to each power.
The sum total of damage distribution must always be 100%, and by default the first power
has 100% of the damage.

There is a built-in limit to how many seconds of damage over time we can track. This is
defined by the constant `max_effect_secs`. Damage over time exceeding this limit is lost.

The main interface to applying damage is the function
`int apply_damage_full(current_effect_second, damage_type, amount, offense, instant, dot)`,
which returns a positive value if the entity died and the value is any leftover (overkill)
damage. You get `current_effect_second` from the `stats` object - it is an index into our
circular damage over time buffer. The `offense` array contains the above statuses but for
offense. The `instant` and `dot` arrays contain fractions of the damage that will also
be dealt as other damage types.

Skills
------

A skill is any ability that an entity (player or non-player) can utilize.

Definitions:
* Skill slots - a number of skills available for use by an entity; an entity must first
  slot a skill before it can be used, which means it becomes configured for use
* Skill windup time - the time from attempting to use a skill until it begins, often
  referred to as 'cast time'; during this time the skill may be interrupted
* Skill animation time - after skill has begun, the entity is locked into a skill
  animation for this given amount of time, unable to start another skill; sometimes this
  is referred to as 'attack speed'
* Skill cooldown time - the time it takes for the skill to become available for use again

Each skill exists in one of these states:
* `skill_state_ready` - default state, skill is ready for use
* `skill_state_windup` - entity is in an interruptible state waiting for skill to start
* `skill_state_windup_done` - we are waiting for higher level code to start the skill
* `skill_state_animation` - skill has begun, spend some time until entity can use a skill
  again
* `skill_state_cooldown` - skill is done and is waiting for its cooldown period to finish

A number of different statuses exist for each:
* `windup_time_modifier` - modifies the windup time
* `animation_time_modifier` - modifies the animation time
* `cooldown_time_modifier` - modifies the cooldown time

Some key skill functions:
* `try_pay_skill(slot)` - only fails if the entity lacks the required power to pay for it
* `try_start_skill(slot)` - try to initiate a skill, includes paying for it as above
* `interrupt()` - call this if the entity is interrupted (eg player decides to move)

Event queue
-----------

Whenever something changes that the calling API should act on, something is pushed to the
'event queue'. The `stats` object contains a member called `entities_with_events` which is
a list of all entities that have pending `events`. The calling API should go through and clear
all of these before the next tick is called. Events are merely informative, although needed for
implementing proper animations, and the only action that the calling API must do is to clear them.
