libdicey
========

A simple and efficient deterministic integer math and pseudo-random roll library,
supporting various types of roll modifiers, such as lucky and mediocre, as well
as different types of randomness, such as roll tables and fair distributions.

Randomness is all based on the `seed` class which maintains the pseudo-random
generator. From this generator you can create random rolls or derive new random
seeds based on input parameters such as coordinates. If you use the same random
seed input, you can recreate the exact same results for the same sequence of
calls.

You can create a new `seed` with a true random value using the `seed_random`
function, like this:

```c++
seed s = seed_random();
```

Note that if you copy the `seed` class by value instead of passing it by
reference, updates to the internal generator state will not be made in the
original. This is sometimes a useful feature, and sometimes not what you
want. So take care. Generally when you want reliable randomness, pass it by
reference, and when you want reproducible state that survives code changes,
pass it by value or rather use a derived function.

Derived functions
-----------------

Derived functions generate random but deterministic outputs from a random seed
depending on their input parameters.

The parameter derived functions could be used for example to deterministically
generate the necessary parts of a random world as they are entered without
having to generate everything at once. The derived values are based on the
original seed value, and not the current state of the pseudo-random generator.

This means that, for example

```c++
seed s(64);
...
seed level799 = s.derive(799);
int value1 = level799.roll(0, 4);
```

will generate the same `level799` and result for `value1` no matter how much
you've used the `s` seed.

Weighted rolls
--------------

If you want a weighted roll on a small number series, where each next outcome is
increasingly unlikely, and you do not want to maintain a roll table for this,
you can use the pow2 and quadratic weighted roll functions instead.

Example:

```c++
seed s(64);
int i = s.pow2_weighted_roll(4);
```
has a 50% probability of returning 0, a 25% probability of return 1, a 12.5%
probability of returning 2, and so on, each following value being half the
likelihood of the previous. `pow2_weighted_roll` only supports values up to 63,
but that is far more than you will ever need or want anyways.

```c++
seed s(64);
int i = s.quadratic_weighted_roll(4);
```
has a bit more than a 1/3th probability of returning 0, less than a third of
returning 1, less than a fourth of 2, a bit more than a tenth of 3 and less
than 1/20th of returning 4. So a less aggressive drop-off in probability
than the pow2 version above.

Roll tables
-----------

Roll tables are the archetypical RPG mechanic of having different weightings
for different outcomes, say you want there to be 4 outcomes of high
probabilities and two outcomes of lower probabilities. Then you can create a
roll table like this:

```c++
seed s(64);
std::vector<int> weightings = { 100, 100, 100, 100, 50, 50 };
roll_table rt(s, weightings);
int result = rt.roll();
```

This gives you a 20% chance of rolling the first result and a 10% chance of
rolling the last result.

You can also roll many results at once with `rolls` and `unique_rolls`. The
latter guarantees that will only get unique results each time. Finally, we
have the `boxgacha` table roll type that removes the result from the table
each time you roll it. All four types can take a `roll_weight` percentage
parameter to improve the roll, where the higher the value the higher the chance
of a less common results.

Linear roll tables
------------------

A linear roll table is different than the normal one in that it only allows
equal probability for each entry, and it guarantees that each result can only
be obtained once until you empty and reset it. As such it simulates a deck of
cards where the roll is a draw where the drawn card goes into a discard pile
and the reset is when you put all the discarded cards back and shuffle. You
can also permanently remove the last drawn entry from the table, and define a
set of entries that are not available but can be added later - this is
similar to tearing/losing a card and adding a new card to your discard deck.

All of these operations except add are O(1) complexity, meaning that
increasing the size of your table does not increase the time they take to
complete. Add and initial construction are O(N), where N for add is the size
of the remaining unused entries.

Example:
```c++
seed s(64);
// Define a 52 card deck with 4 cards unavailable from the start
linear_roll_table cards(s, 52, table_reset_policy::reset, 48);
int result = cards.roll(); // draw a card, can be card 0 to 47
cards.add(48); // add new card 48 to the discard pile
cards.reset(); // shuffle in the new card
int result = cards.roll(); // draw a card, can be card 0 to 48
cards.remove(); // permanently remove the card we just drew
```

Luck
----

Some functions take a `luck_type` parameter. This is a quick way to add some
ways to manipulate your rolls.

A `lucky` roll gives you the best of two rolls, an `unlucky` gives you the
worst of two, while the `very_lucky` and `very_unlucky` gives you best and
worst of three rolls. The more special `mediocre` roll type will give you
the least extreme of two rolls, while `uncommon` roll types will give you
the most extreme of two rolls, where extreme means the furthest from the
average.

You can combine opposed luck types with `luck_combine`, if for example one
player has one luck type to hit and one enemy has another luck type to avoid
being hit, and it tries to give you the sensible result for the entity doing
the roll.

Distribution
------------

Sometimes you want outcomes that look random but also want to avoid sequences
of outcomes that might be decidedly unfun. For example if you are looking for
the random key drop in a chest with a 5% probability, you would expect to hit
it after opening 25 chests, but every 200th player out there is going to be
unlucky enough to open 100 chests and still not get one; and might complain
loudly or quit. Similarly, the lucky one who gets it on his first chest
opening might have an unfair advantage or breeze through the content too fast.
This is when you want a pseudo-random distribution rather than pseudo-random
rolls.

The probabilities we count toward are in permille, and you can choose between
three different methods to count: `relaxed` which is random and fair over
longer number sequences, but can give you two successes back-to-back. `fair`
which is more strict and guarantees that the success will only be rolled once
inside 50% of the average expectancy of the outcome; so for example if it has
a 10% chance, it will always occur sometime between the 50th and the 150th roll.
`predictable` which can be used when the only important thing is that the results
are well distributed, with equal time between successes, and predictability is
not important; the only random part is the first result, which will always occur
before the average time to success.

To use pseudo-random distribution you need to maintain a `prd` object for your
game entity for each type of roll. Example:

```c++
seed s(64);
prd find_key(s, 50); // 5% chance of success
...
if (find_key.roll()) success();
```

Rolling on a `prd` is fast as rolling on the seed class but each `prd` instance
takes up 16 bytes.

Implementations
---------------

These are test implementations contained in this repository that will likely
be spun out at some point in the future:

* [ARPG stats library](doc/arpgstats.md)
* [Chunky dungeon digger library](doc/chunky.md)
