ARPG Stats Library
------------------

This is a library for doing some of the internal game state calculations for an action
role-playing game (ARPG). It can take care of tracking powers (such as health, stamina,
energy), skill cooldowns, buffs and debuffs, while implementing features such as
regeneration, temporary bonus power, and temporary and permanent upgrades.

The main C++ header defines a section dedicated to game wide setup state. You are meant
to modify these to match your game.

The library is written to be fast and there are some sacrifices made to ensure that
this is the case.

Powers
======

We define a 'power' as a finite resource of some kind that the entity uses
or needs and can replenish. For example health, mana, stamina, or energy. You
should set the power_types constant to the number of powers you will use.

Internally arpgstats stores the amount as a fraction of 1024. You can use this
amount directly or use it as a fraction to derive your own values.

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

Damage
======

Damage is any kind of loss of power that is not a skill cost. We support any number
of different damage types.

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

There is a built-in limit to how many seconds of damage over time we can track. This is
defined by the constant `max_effect_secs`. Damage over time exceeding this limit is lost.

Skills
======

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
* `interrupt_ignore_chance` - chance to ignore any interrupts during windup stage

Some key skill functions:
* `try_pay_skill(slot)` - only fails if the entity lacks the required power to pay for it
* `try_start_skill(slot)` - try to initiate a skill, includes paying for it as above
* `try_interrupt_skill(slot)` - call this if the skill might be interrupted for any reason
