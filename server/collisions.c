#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "globals.h"

static void addInertia(entity *from, contact *c, int addedI) {
	contact *crnt;
	double *vec = c->vec;
	double dot;
	int i;
	for (i = from->numContacts - 1; i >= 0; i--) {
		crnt = from->contacts + i;
		dot = crnt->vec[0]*vec[0] + crnt->vec[1]*vec[1];
		if (dot >= 0) continue;
		int added = -dot * addedI;
		if (added) {
			crnt->other->contacts[crnt->twin].inertia += added;
			addInertia(crnt->other, crnt->other->contacts + crnt->twin, added);
		}
	}
}

//TODO: Decide if we want to store the dot product of each contct's vec with each other
static contact* addContactSub(entity *from, entity *to, double *vec) {
	if (from->numContacts == from->maxContacts) {
		from->maxContacts += 3;
		from->contacts = realloc(from->contacts, sizeof(contact) * from->maxContacts);
	}
	contact *c = from->contacts + (from->numContacts++);
	c->other = to;
	memcpy(c->vec, vec, sizeof(double) * 2);
	c->inertia = to->me->r;

	int i;
	contact *crnt; // current
	double dot;
	for (i = to->numContacts - 1; i >= 0; i--) {
		crnt = to->contacts + i;
		dot = crnt->vec[0]*vec[0] + crnt->vec[1]*vec[1];
		if (dot > 0)
			c->inertia += crnt->inertia * dot;
	}
	return c;
}

static contact* addContact(entity *a, entity *b, int64_t *pos) {
	(*a->myAi->handleCollision)(a, b);
	(*b->myAi->handleCollision)(b, a);
	double vec[2];
	double dist = sqrt(pos[0]*pos[0] + pos[1]*pos[1]);
	int dim;
	for (dim = 0; dim < 2; dim++) {
		vec[dim] = pos[dim] / dist;
	}
	contact *ab = addContactSub(a, b, vec);
	for (dim = 0; dim < 2; dim++) {
		vec[dim] *= -1;
	}
	contact *ba = addContactSub(b, a, vec);
	ab->twin = ba - b->contacts;
	ba->twin = ab - a->contacts;
	int tmp = ba->inertia;
	addInertia(a, ab, ab->inertia);
	addInertia(b, ba, tmp);
	return ab;
}

static entity **collisions1 = NULL, **collisions2 = NULL;
static int numCol2; // This has to be global so that our helper methods can get at it easier

static void moveDammit(contact *c, int32_t v, int iteration) {
	entity *him = c->other;
	if (iteration > him->lastIteration) {
		him->lastIteration = iteration;
		collisions2[numCol2++] = him;
	}
	guarantee *target = him->me;
	int dim;
	for (dim = 0; dim < 2; dim++) {
		target->vel[dim] += (int)(v * c->vec[dim]);
	}
	guaranteeMoved(target, 0);

	int i;
	double *vec = c->vec;
	double dot;
	contact *c2;
	for (i = him->numContacts - 1; i >= 0; i--) {
		if (i == c->twin) continue; // Because the dot product would be -1
		c2 = him->contacts + i;
		dot = 0;
		for (dim = 0; dim < 2; dim++) {
			dot += vec[dim] * c2->vec[dim];
		}
		if (dot <= 0) continue;
		dim = dot * v; // I realize this is an egregious misuse of a variable named 'dim', but it's only for 3 lines...
		if (dim)
			moveDammit(c2, dim, iteration);
	}
}

static void hitEachOther(entity *a, entity *b, int32_t *vel, int iteration, int64_t *pos) {
	int i;
	contact *c;
	for (i = a->numContacts - 1; i >= 0; i--) {
		if (a->contacts[i].other == b) {
			c = a->contacts + i;
			break;
		}
	}
	if (i < 0)
		c = addContact(a, b, pos);
	double deltaV = 3; // This way, no matter what kind of integer sh*t and split we get, something always moves
	for (i = 0; i < 2; i++) {
		deltaV += vel[i] * c->vec[i];
	}
	int sumInertia = c->inertia + c->other->contacts[c->twin].inertia;
	int32_t v1 = deltaV * c->inertia / sumInertia + 0.5;
	if (v1)
		moveDammit(c->other->contacts + c->twin, v1, iteration);
	if (v1 != deltaV)
		moveDammit(c, deltaV - v1, iteration);
}

void doStep() {
	int numCol1 = 0;
	int dim, i;
	int64_t r;
	guarantee *g, *o;

	entity *reader;
	sector *sec;
	for (sec = listrootsector; sec; sec = sec->nextsector) {
		for (reader = sec->firstentity; reader; reader = reader->next) {
			reader->numContacts = 0;
			reader->lastIteration = 0;
			numCol1++; // TODO: Keep track (in the sector) of the number of entities
		}
	}
	collisions1 = realloc(collisions1, numCol1 * sizeof(entity*));
	collisions2 = realloc(collisions2, numCol1 * sizeof(entity*));
	numCol1 = 0;
	for (sec = listrootsector; sec; sec = sec->nextsector) {
		for (reader = sec->firstentity; reader; reader = reader->next) {
			collisions1[numCol1++] = reader;
		}
	}
	int64_t pos[2];
	int32_t vel[2];
	double speed;
	double dvel[2];
	int iteration = 0;
	while (numCol1) {
		iteration++;
		numCol2 = 0;
		for (numCol1--; numCol1 >= 0; numCol1--) {
			g = collisions1[numCol1]->me; // TODO: Store the guarantees, not references to them, in the entities
			for (i = g->numIntersects - 1; i >= 0; i--) {
				o = g->intersects[i];
				if (o->ent == NULL) continue;
				for (dim = 0; dim < 2; dim++) {
					vel[dim] = g->vel[dim] - o->vel[dim];
					pos[dim] = o->pos[dim] - g->pos[dim] + POS_RANGE * (o->spos[dim] - g->spos[dim]);
				}
				speed = sqrt(vel[0]*vel[0] + vel[1]*vel[1]);
				if (speed == 0) continue;
				for (dim = 0; dim < 2; dim++)
					dvel[dim] = vel[dim] / speed;
				double dist = 0;
				for (dim = 0; dim < 2; dim++) {
					dist += pos[dim] * dvel[dim];
				}
				if (dist <= 0) continue;
				r = g->r + o->r;
				if (speed + r <= dist) continue;
				if (speed < dist) dist = speed;
				double dispSqrd = 0;
				for (dim = 0; dim < 2; dim++) {
					double delta = pos[dim] - dist*dvel[dim];
					dispSqrd += delta * delta;
				}
				if (dispSqrd < r * r) {
					hitEachOther(g->ent, o->ent, vel, iteration, pos);
				}
			}
		}
		entity **colHolder = collisions2;
		collisions2 = collisions1;
		collisions1 = colHolder;
		numCol1 = numCol2;
	}

	for (sec = listrootsector; sec; sec = sec->nextsector) {
		for (reader = sec->firstentity; reader; reader = reader->next) {
			g = reader->me;
			g->pos[0] += g->vel[0];
			g->pos[1] += g->vel[1];
			guaranteeMoved(g, 1);
		}
	}
}
