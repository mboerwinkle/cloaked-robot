#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "globals.h"

/*static int debugCounter = 0;
#define pred ((debugCounter == 444 || debugCounter == 457) && (int64_t)g == 0x713a00)

static void debugMessage(guarantee *g, char *msg) {
	if (g != 0x713a00) return;
	debugCounter++;
	if (debugCounter < 444) return;
	puts(msg);
	int i;
	for (i = 0; i < g->numKids; i++) {
		printf("\t%p (%d, %d) <%d, %d>, size %d\n", g->kids[i], g->kids[i]->pos[0], g->kids[i]->pos[1], g->kids[i]->vel[0], g->kids[i]->vel[1], g->kids[i]->r);
	}
	if (!strcmp("I fit", msg)) {
		puts("We'll see about that!");
	}
}*/

static void myExit(int code) {
	puts("Shoulda stopped me!");
	exit(code);
}

static char fitGuarantee(guarantee *g);

static char checkGuarantee(guarantee *kid) {
	guarantee *p = kid->parent;
	if (p == NULL) {
		kid->pto = 0;
		return 0;
	}
	if (kid->pto + kid->T > p->T) return 2;
	int dim = 0;
	for (; dim < 2; dim++) {
		if (abs(p->pos[dim] + kid->pto*p->vel[dim] - kid->pos[dim]) > p->r - kid->r)
			return 1;
		if (abs(p->pos[dim] + (kid->pto+kid->T) * p->vel[dim] - kid->pos[dim] - kid->vel[dim] * kid->T) > p->r - kid->r)
			return 1;
	}
	return 0;
}

static void calcSlope(int dim, char left, guarantee *g, int32_t time, int32_t space, int32_t baseSpace, int baseSlope, int32_t *nextT, int32_t *nextS, int *slope, int32_t *loss) {
	if (left && time == 0) {
		*slope = baseSlope;
		*loss = 0;
		*nextT = -1;
		*nextS = space - baseSlope;
		return;
	}
	guarantee **kids = g->kids;
	*slope = INT_MIN;
	int k = g->numKids - 1;
	for (; k >= 0; k--) {
		int32_t T = kids[k]->T + kids[k]->pto;
		if (left ? (T < time) : (T > time)) {
			int32_t S = kids[k]->pos[dim] + kids[k]->vel[dim] * kids[k]->T + (left ? -kids[k]->r : kids[k]->r);
			//All of the differences in the next two lines really ought to be switched if we're on the right, but because of the nature of '/' and '%' we can get away with what we're doing.
			int mySlope = (space - S) / (time - T); //Except we want this rounded... down. Even if negative.
			int32_t myLoss = (left ? space - S : S - space) % (time - T);
			if (myLoss < 0) { // TODO: Thoroughly verify that changing this from <= to < wasn't a terrible mistake!
				mySlope--;
				myLoss += left ? (time - T) : (T - time);
			}
			if (mySlope >= *slope && (mySlope > *slope || myLoss > *loss)) {
				*slope = mySlope;
				*loss = myLoss;
				*nextS = S;
				*nextT = T;
			}
		}
	}
	if (left && time) {
		if (time == -1) return;
		int mySlope = (space - baseSpace) / time;
		int32_t myLoss = (space - baseSpace) % time;
		if (myLoss < 0) {
			mySlope--;
			myLoss += time;
		}
		if (mySlope >= *slope && (mySlope > *slope || myLoss >= *loss)) {
			*slope = mySlope;
			*loss = myLoss;
			if (baseSlope >= mySlope && (baseSlope > mySlope || myLoss == 0)) {
				*nextT = -1;
				*nextS = baseSpace - mySlope;
			} else {
				*nextS = baseSpace;
				*nextT = 0;
			}
		}
	}
}

static char fitGuarantee(guarantee *g) {
	if (g->numKids == 0) {
		puts("A childless guarantee is a sad thing indeed.");
		myExit(5);
	}
	guarantee **kids = g->kids;
	int dim = 0;
	int32_t leftS, leftT;
	int32_t rightS, rightT;
	int32_t leftBaseS;
	int leftBaseSlp = 0, rightBaseSlp = 0;
	int k;
	for (; dim < 2; dim++) {
		leftT = rightT = 0;
		leftS = leftBaseS = INT32_MAX;
		rightS = INT32_MIN;
		for (k = g->numKids-1; k >= 0; k--) {
			int32_t T = kids[k]->T + kids[k]->pto;
			if (T >= leftT) {
				int32_t S = kids[k]->pos[dim] + kids[k]->vel[dim] * kids[k]->T - kids[k]->r;
				if (T > leftT) {
					leftT = T;
					leftS = S;
				} else if (S < leftS) {
					leftS = S;
				}
			}
			if (T < 0 || kids[k]->pto > 0) {
				puts("wtfuuuuuuuu");
				myExit(6);
			}
			int32_t S = kids[k]->pos[dim] - kids[k]->pto * kids[k]->vel[dim] + kids[k]->r;
			if (S > rightS) {
				rightS = S;
				rightBaseSlp = kids[k]->vel[dim];
			}
			S -= 2*kids[k]->r;
			if (S < leftBaseS) {
				leftBaseS = S;
				leftBaseSlp = kids[k]->vel[dim];
			}
		}
		/*if (pred) {
			printf("On the way out, here's what we've got:\n"
			"leftT: %d\tleftS: %d\n"
			"rightS: %d\n"
			"leftBaseS: %d\tleftBaseSlp: %d\n"
			"rightBaseSlp: %d\n", leftT, leftS, rightS, leftBaseS, leftBaseSlp, rightBaseSlp);
		}*/
		int32_t lNextS, rNextS;
		int32_t lNextT, rNextT;
		int lSlope, rSlope;
		int32_t lLoss, rLoss;
		//if (deleteMe == 2) printf("T: %d, S: %d\n", leftT, leftS);
		calcSlope(dim, 1, g, leftT, leftS, leftBaseS, leftBaseSlp, &lNextT, &lNextS, &lSlope, &lLoss);
		calcSlope(dim, 0, g, rightT, rightS, 0, 0, &rNextT, &rNextS, &rSlope, &rLoss);
		/*if (rSlope <= rightBaseSlp && (rSlope < rightBaseSlp || rLoss > 0)) {
			rSlope = rightBaseSlp;
			rLoss = 0;
			rNextS = rightS;
			rNextT = rightT;
			if (pred) puts("Did the thing");
		} else {
			if (pred) puts("Didn't do the thing");
		}
		rightS -= rSlope;
		rightT -= 1;*/
		while (1) {
			if (lSlope > rSlope) {
				//if (lNextT <= rightT) {
				if (lNextT < rightT) {
					if (lLoss <= leftT - rightT) {
						leftT = lNextT;
						leftS = lNextS;
						break;
					}
					lSlope++;
					break;
				}
				leftT = lNextT;
				leftS = lNextS;
				//if (pred) printf("Left side stepped to\n  S: %d\tT: %d\n", leftS, leftT);
				calcSlope(dim, 1, g, leftT, leftS, leftBaseS, leftBaseSlp, &lNextT, &lNextS, &lSlope, &lLoss);
			} else if (rSlope > lSlope) {
				//if (leftT <= rNextT) {
				if (leftT < rNextT) {
					if (rLoss <= leftT - rightT) {
						rightT = rNextT;
						rightS = rNextS;
						lSlope = rSlope; // Because the value 'returned' by this chunk of code is read from lSlope
						break;
					}
					lSlope = rSlope + 1;
					break;
				}
				rightT = rNextT;
				rightS = rNextS;
				//if (pred) printf("Right side stepped to\n  S: %d\tT: %d\n", rightS, rightT);
				calcSlope(dim, 0, g, rightT, rightS, 0, 0, &rNextT, &rNextS, &rSlope, &rLoss);
			} else { // Else, we're going to have both bottlenecks changing on us at once
				//if (lNextT <= rNextT) {
				if (lNextT < rNextT) {
					if (lLoss + rLoss <= leftT - rightT) {
						rightT = rNextT;
						rightS = rNextS;
						leftT = lNextT;
						leftS = lNextS;
						break;
					}
					lSlope++;
					break;
				}
				leftT = lNextT;
				leftS = lNextS;
				rightT = rNextT;
				rightS = rNextS;
				/*if (pred) {
					printf("Left side stepped to\n  S: %d\tT: %d\n", leftS, leftT);
					printf("AND Right side stepped to\n  S: %d\tT: %d\n", rightS, rightT);
				}*/
				calcSlope(dim, 1, g, leftT, leftS, leftBaseS, leftBaseSlp, &lNextT, &lNextS, &lSlope, &lLoss);
				calcSlope(dim, 0, g, rightT, rightS, 0, 0, &rNextT, &rNextS, &rSlope, &rLoss);
			}
		}
		//if (pred) printf("End with:\n  Slope: %d\n  leftS: %d\tleftT: %d\n  rightS: %d\nrightT: %d\n", lSlope, leftS, leftT, rightS, rightT);
		g->pos[dim] = (rightS - lSlope * rightT + leftS - lSlope * leftT) / 2;
		if (g->pos[dim] + lSlope * leftT - leftS > g->r ||
			rightS - g->pos[dim] - lSlope * rightT > g->r) {
			//debugMessage(g, "I don't fit");
			return 1;
		}
		g->vel[dim] = lSlope;
		char lt = 0, gt = 0;
		for (k = g->numKids - 1; k >= 0; k--) {
			if (g->kids[k]->vel[dim] <= lSlope) lt = 1;
			if (g->kids[k]->vel[dim] >= lSlope) gt = 1;
		}
		if (!(lt && gt)){
			printf("%d, dim: %d, slope: %d\n", g->r, dim, lSlope);
			printf("lS: %d, lSlp: %d, rSlp: %d\n", leftBaseS, leftBaseSlp, rightBaseSlp);
			for (k = g->numKids - 1; k >= 0; k--) {
				printf("%d at %d by %d\n", g->kids[k]->r, g->kids[k]->pos[dim], g->kids[k]->vel[dim]);
				printf("for %d starting at %d\n", g->kids[k]->T, g->kids[k]->pto);
			}
			puts("Wicked, wicked");
			myExit(0);
		}
		//if (abs(lSlope) >= 10000) printf("%d\n", lSlope);
	}
	//debugMessage(g, "I fit");
	return 0;
}

//Removes dude from g's list of intersects. One way, not both.
static void removeIntersect(guarantee *dude, guarantee *g) {
	int i = --g->numIntersects;
	for (; i >= 0; i--) {
		if (dude == g->intersects[i]) {
			g->intersects[i] = g->intersects[g->numIntersects];
			g->ito[i] = g->ito[g->numIntersects];
			return;
		}
	}
	puts("Can't remove what ain't there...");
	myExit(10);
}

static void removeAllIntersects(guarantee *g) {
	int i = g->numIntersects - 1;
	for (; i >= 0; i--) {
		removeIntersect(g, g->intersects[i]);
	}
	g->numIntersects = 0;
}

static void addIntersect(guarantee *a, guarantee *b, int32_t ito) {
	if (a->numIntersects == a->maxIntersects) {
		a->maxIntersects += 5;
		a->intersects = realloc(a->intersects, sizeof(guarantee*) * a->maxIntersects);
		a->ito = realloc(a->ito, sizeof(int32_t) * a->maxIntersects);
	}
	a->intersects[a->numIntersects] = b;
	a->ito[a->numIntersects] = ito;
	a->numIntersects++;
}

//Hope you've done something with its kids!
void destroyGuarantee(guarantee *g) {
//	debugMessage(g, "Destroyed :(");
	removeAllIntersects(g);
	free(g->kids);
	free(g->intersects);
	free(g->ito);
	guarantee *p = g->parent;
	if (p) {
		int i;
		for (i = --(p->numKids); i >= 0; i--) {
			if (p->kids[i] == g) {
				p->kids[i] = p->kids[p->numKids];
				break;
			}
		}
		if (i < 0) {
			puts("The body's gone, Pa!");
			myExit(16);
		}
		if (p->numKids == 0) {
			destroyGuarantee(p);
		}
	} else {
		g->sec->topGuarantee = NULL;
	}
	free(g);
}

static void moreKids(guarantee *g, int max) {
//	debugMessage(g, "gets more kids");
	g->kids = realloc(g->kids, sizeof(guarantee*) * max);
}

static char checkIntersect(guarantee *a, guarantee *b, int32_t ito) {
	int32_t IoIL = 0;
	int32_t IoIU = a->T; // Interval of Intersection, Lower and Upper. All times are in a's reference frame
	if (b->T - ito < IoIU)
		IoIU = b->T - ito;
	int32_t origIoIU = IoIU;
	int dim = 0;
	int32_t r = a->r + b->r;
	for (; dim < 2; dim++) {
		int32_t dpos = b->pos[dim] + b->vel[dim] * ito - a->pos[dim] + POS_RANGE * (b->spos[dim] - a->spos[dim]);
		int32_t dvel = a->vel[dim] - b->vel[dim];
		if (dvel == 0) {
			if (abs(dpos) < r)
				continue;
			break;
		}
		if (dvel < 0) {
			dpos *= -1;
			dvel *= -1;
		}
		if (r <= -dpos)
			break;

		int32_t t = (r + dpos - 1) / dvel;
		if (t < IoIU) IoIU = t;

		if (r <= dpos) {
			t = (dpos - r) / dvel + 1;
			if (t > IoIL) IoIL = t;
		}

		if (IoIU < IoIL) break;
	}
	if (dim < 2) //We quit prematurely, because no intersection was possible
		return -1;
	if (IoIL == 0 && IoIU == origIoIU && a->ent == NULL && b->ent == NULL && a->T <= b->T && a->parent == b->parent && a->parent != NULL) { // Do we *need* them to be siblings?
		int i;
		for (i = b->numKids - 1; i >= 0; i--) {
			if (b->kids[i]->T > a->T)
				break;
		}
		if (i < 0) {
		//	debugMessage(a, "Tries to get kids");
		//	debugMessage(b, "Tries to lose kids");
			int newNumKids = a->numKids + b->numKids;
			if (newNumKids > a->maxKids) {
				moreKids(a, newNumKids);
				a->maxKids = newNumKids;
			}
			guarantee** adoptedKids = a->kids + a->numKids;
			memcpy(adoptedKids, b->kids, sizeof(guarantee*) * b->numKids);
			for (i = b->numKids - 1; i >= 0; i--) {
				adoptedKids[i]->pto -= ito;
				adoptedKids[i]->parent = a;
			}
			int32_t oldPos[2];
			int32_t oldVel[2];
			memcpy(oldPos, a->pos, sizeof(oldPos));
			memcpy(oldVel, a->vel, sizeof(oldVel));
			int oldNumKids = a->numKids;
			a->numKids = newNumKids;
			if (fitGuarantee(a)) {
				for (i = b->numKids - 1; i >= 0; i--) {
					adoptedKids[i]->pto += ito;
					adoptedKids[i]->parent = b;
				}
				memcpy(a->pos, oldPos, sizeof(oldPos));
				memcpy(a->vel, oldVel, sizeof(oldVel));
				a->numKids = oldNumKids;
			//	debugMessage(a, "Had to give them back");
			//	debugMessage(b, "Had to take them back");
			} else {
			//	debugMessage(a, "Got 'em!");
				destroyGuarantee(b);
				guaranteeMoved(a, 0);
				return 0;
			}
		}
	}
	addIntersect(a, b, ito);
	addIntersect(b, a, -ito);
	return 1;
}

static void checkRecursive(guarantee *g, guarantee *o, int32_t offset) {
	if (1 != checkIntersect(g, o, offset)) return;
	int i;
	for (i = o->numKids - 1; i >= 0; i--) {
		checkRecursive(g, o->kids[i], offset - o->kids[i]->pto);
	}
}

static void getIntersects(guarantee *g) {
	guarantee *p = g->parent;
	//At some point, we may want to make this so that we don't remove and read the listings for the guys we're *still* intersecting
	removeAllIntersects(g);
	addIntersect(g, g, 0);
	if (p == NULL) {
		sector *sec;
		for (sec = listrootsector; sec; sec = sec->nextsector) {
			if (sec->topGuarantee == NULL) continue;
			if (g == sec->topGuarantee) continue;
			checkIntersect(g, sec->topGuarantee, g->pto - sec->topGuarantee->pto);
		}
		return;
	}
	int i, j;
	int32_t pto = g->pto;
	guarantee **hisKids;
	for (i = p->numIntersects - 1; i >= 0; i--) {
		int32_t uto = pto + p->ito[i]; // Uncle Time Offset, even though it isn't acutally an uncle - p and it aren't siblings, they're intersects.
		if (p->intersects[i]->ent) {
			checkIntersect(g, p->intersects[i], uto);
			continue;
		}
		hisKids = p->intersects[i]->kids;
		for (j = p->intersects[i]->numKids - 1; j >= 0; j--) {
			if (g == hisKids[j]) {
				continue;
			}
			if (g->ent) {
				checkRecursive(g, hisKids[j], uto - hisKids[j]->pto);
			} else {
				if (0 == checkIntersect(g, hisKids[j], uto - hisKids[j]->pto))
					return;
			}
		}
	}
}

static int calcRank(int32_t r) {
	uint32_t i = 1;
	int rank = 0;
	while (i < r) {
		i *= 4;
		rank++;
	}
	return rank;
}

static void genParent(guarantee *who) {
	if (who->sec) { // TODO: Delete this, it's for debugging
		printGs(NULL, who);
		puts("!!!!!!!!!!!!!");
	}
	guarantee *parent = malloc(sizeof(guarantee));
//	debugMessage(parent, "Created as a parent");
//	debugMessage(who, "Has a parent generated for me");
	parent->numIntersects = parent->maxIntersects = 1;
	parent->intersects = malloc(sizeof(guarantee*));
	parent->ito = malloc(sizeof(int32_t));
	parent->intersects[0] = parent;
	parent->ito[0] = 0;
	parent->maxKids = 2;
	parent->numKids = 1;
	parent->kids = malloc(sizeof(guarantee*) * 2);
	parent->kids[0] = who;
	parent->parent = NULL;
	memcpy(parent->spos, who->spos, sizeof(parent->spos));
	memcpy(parent->pos, who->pos, sizeof(parent->pos));
	//memcpy(parent->old, who->pos, sizeof(parent->pos));
	//memset(parent->trend, 0, sizeof(parent->trend));
	memcpy(parent->vel, who->vel, sizeof(parent->vel));
	parent->pto = 0;
	parent->ent = NULL;
	parent->sec = NULL;
	parent->T = who->T * 2;
	parent->r = 1 << 2 * (1 + calcRank(who->r));
	who->parent = parent;
}

static void addSibling(guarantee *brother, guarantee *newborn) {
//	debugMessage(brother, "Got a sibling");
//	debugMessage(newborn, "Given a sibling");
	if (!brother->parent) {
		puts("I feel like this is a problem.");
		genParent(brother);
		newborn->parent = brother->parent;
	}
	guarantee *parent = brother->parent;
//	debugMessage(parent, "Got a kid");
	if (parent->numKids == parent->maxKids) {
		moreKids(parent, parent->maxKids += 2);
	}
	parent->kids[parent->numKids] = newborn;
	newborn->parent = parent;
	parent->numKids++;
}

static char sortingHat(guarantee *who, guarantee *old, guarantee *new) {
//	debugMessage(who, "Gets sorted");
	int32_t deltaR = new->r - who->r;
	int32_t T = who->T + who->pto;
	int dim = 0;
	for (; dim < 2; dim++) { // Before, the lower block of code referrenced 'trend'
		int32_t pos = who->pos[dim] - who->pto * who->vel[dim];
		if (abs(pos - new->pos[dim]) > deltaR)
			return 0;
		pos = who->pos[dim] + who->vel[dim] * who->T;
		if (abs(pos - new->pos[dim] - new->vel[dim] * T) > deltaR)
			return 0;
	}
	char retVal = 0;
	int32_t worstFail = 0;
#define sortMacro(r) {\
	if (diff > worstFail) {\
		worstFail = diff;\
		retVal = r;\
	}\
}
	for (dim = 0; dim < 2; dim++) {
		int32_t pos = who->pos[dim] - who->pto * who->vel[dim];
		int32_t diff = abs(pos - new->pos[dim]);
		sortMacro(0);
		diff = abs(pos - old->pos[dim]);
		sortMacro(1);

		pos = who->pos[dim] + who->vel[dim] * who->T;
		diff = abs(pos - new->pos[dim] - new->vel[dim] * T);
		sortMacro(0);
		diff = abs(pos - old->pos[dim] - old->vel[dim] * T);
		sortMacro(1);
	}
	//printf("%d\n", retVal);
	return retVal;
}

guarantee* createEntityGuarantee(guarantee *creator, sector *sec, int32_t r, int32_t *pos, int32_t *vel, entity *ent) {
	guarantee *ret = malloc(sizeof(guarantee));
//	debugMessage(ret, "Created as a guarantee for an ent");
	ret->maxIntersects = 1;
	ret->numIntersects = 0;
	ret->intersects = malloc(sizeof(guarantee*));
	ret->ito = malloc(sizeof(int32_t));
	ret->r = r;
	ret->spos[0] = sec->x;
	ret->spos[1] = sec->y;
	//memcpy(ret->old, pos, sizeof(ret->old));
	memcpy(ret->pos, pos, sizeof(ret->pos));
	memcpy(ret->vel, vel, sizeof(ret->vel));
	//memcpy(ret->trend, vel, sizeof(ret->trend));
	ret->ent = ent;
	ret->T = 1;
	ret->numKids = ret->maxKids = 0;
	ret->kids = NULL;
	ret->sec = NULL;
	int myRank = calcRank(r);
	if (creator == NULL) {
		//puts("Starting a new world, it seems");
		ret->parent = NULL;
		ret->pto = 0;
		creator = ret;
		for (; myRank < 14; myRank++) {
			genParent(creator);
			creator = creator->parent;
		}
		creator->sec = sec;
		sec->topGuarantee = creator;
	} else {
		int hisRank = calcRank(creator->r);
		if (myRank > hisRank) {
			ret->pto = creator->pto;
			for (; myRank > hisRank; hisRank++) {
				if (creator->parent == NULL)
					genParent(creator);
				creator = creator->parent;
				ret->pto += creator->pto;
			}
		} else if (myRank < hisRank) {
			for (; myRank < hisRank; myRank++) {
				ret->pto = 0;
				genParent(ret);
				ret = ret->parent;
			}
			ret->pto = creator->pto;
		} else {
			ret->pto = creator->pto;
		}

		addSibling(creator, ret);
		while (ret->ent == NULL) {
			guaranteeMoved(ret, 0);
			ret = ret->kids[0];
		}
		guaranteeMoved(ret, 0);
	}
	return ret;
}

void guaranteeMoved(guarantee *g, int32_t time) {
//	debugMessage(g, "Moved");
	g->pto += time;
	/*if (time) {
		if (updateTrend) {
			int dim = 0;
			for (; dim < 3; dim++) {
				g->trend[dim] = (g->pos[dim] - g->old[dim]) / time;
			}
		}
		memcpy(g->old, g->pos, sizeof(g->pos));
	}*/
	int ret;
	if ( (ret = checkGuarantee(g)) ) {
	//	debugMessage(g, "Broke parent bounds");
		//Otherwise, we've broken the bounds, and it's time to craft a new parent guarantee
		guarantee *p = g->parent;
	//	debugMessage(p, "Kid broke my bounds");
		guarantee **siblings = p->kids;
		int i = p->numKids - 1;
		/*for (; i >= 0; i--) {
			if (siblings[i] != g) {
				if (checkGuarantee(siblings[i])) {
					puts("nopenopenopenope");
					myExit(21);
				}
			}
		}*/
		int32_t pto = g->pto;
		if (pto < 0) {
			puts("Narp!");
			myExit(14);
		}
		i = p->numKids - 1;
		for (; i >= 0; i--) {
			siblings[i]->pto -= pto;
		}
		//guarantee *new2 = NULL;
		if (ret == 2) { // If it was a time violation
			p->T *= 2;
			if (p->T > 150)
				p->T = 150; // 3 sec @ 50 Hz
		} else {
			//int32_t oldT = p->T;
			p->T -= (p->T - g->T - pto) / 3;
			//int numKids = 0;
			//guarantee **kids = malloc(sizeof(guarantee*) * p->maxKids);
			for (i = p->numKids - 1; i >= 0; i--) {
				if (siblings[i]->T > p->T) {
					//kids[numKids++] = siblings[i];
					//siblings[i] = siblings[--(p->numKids)];
					p->T = siblings[i]->T;
				}
			}
			/*if (numKids) {
				new2 = malloc(sizeof(guarantee));
				memcpy(new2, p, sizeof(guarantee));
				new2->intersects = malloc(sizeof(guarantee*) * p->maxIntersects);
				new2->ito = malloc(sizeof(int32_t) * p->maxIntersects);
				new2->numIntersects = 0;
				new2->kids = kids;
				new2->numKids = numKids;
				new2->T = oldT;
				for (numKids--; numKids >= 0; numKids--) {
					kids[numKids]->parent = new2;
				}
				if (fitGuarantee(new2)) {
					puts("YOU\n\tSHALL NOT\n\t\tPASS");
					myExit(15);
				}
			} else {
				free(kids);
			}*/
		}
		//I would also take this opportunity to recalculate all of the parent's ito's, but I know they're just going to be done over when we call "guaranteeMoved(p)"
		int32_t oldPos[2];
		int32_t oldVel[2];
		memcpy(oldPos, p->pos, sizeof(p->pos));
		memcpy(oldVel, p->vel, sizeof(p->vel));
		if (fitGuarantee(p)) {
		//	debugMessage(p, "Gotta split");
			if (p->numKids == 1) {
				puts("wtf no");
				myExit(102);
			}
			memcpy(p->pos, oldPos, sizeof(oldPos));
			memcpy(p->vel, oldVel, sizeof(oldVel));
			guarantee *new = malloc(sizeof(guarantee));
			memcpy(new, p, sizeof(guarantee));
			new->intersects = malloc(sizeof(guarantee*) * p->maxIntersects);
			new->ito = malloc(sizeof(int32_t) * p->maxIntersects);
			new->numIntersects = 0;
			new->kids = malloc(sizeof(guarantee*) * p->maxKids);
			new->kids[0] = g;
			g->parent = new;
			new->numKids = 1;
			if (fitGuarantee(new)) {
				puts("Boly Shit, Hatman!");
				myExit(11);
			}
			i = p->numKids - 1;
			for (; i >= 0; i--) {
				if (siblings[i] == g) {
					siblings[i] = siblings[--(p->numKids)];
					continue;
				}
				if (sortingHat(siblings[i], p, new)) {
					new->kids[new->numKids++] = siblings[i];
					siblings[i]->parent = new;
					siblings[i] = siblings[--(p->numKids)];
				}
			}

			if (fitGuarantee(p)) {
				puts("Cholesterol levels critical!!");
				myExit(12);
			}
			guaranteeMoved(p, pto);

			addSibling(p, new);
			new->pto = p->pto - pto;
			if (fitGuarantee(new)) {
				puts("...Chicken Butt!");
				myExit(13);
			}
			guaranteeMoved(new, pto);
		} else {
			guaranteeMoved(p, pto);
		}
		/*if (new2) {
			addSibling(p, new2);
			new2->pto = p->pto - pto;
			guaranteeMoved(new2, pto);
		}*/
	}
	getIntersects(g);
}

void printGs(char *prefix, guarantee *g) {
	char s = '\0';
	if (prefix == NULL) prefix = &s;
	printf("%s(%6d, %6d) <%6d, %6d> @%d %c%c%c %lX)", prefix, g->pos[0], g->pos[1], g->vel[0], g->vel[1], calcRank(g->r), g->ent?'#':'-', g->parent?' ':'&', g->sec?'&':' ', ((long int)g)/sizeof(guarantee));
	int i;
	for (i = g->numIntersects - 1; i >= 0; i--) {
		printf(":%lX", ((long int)g->intersects[i])/sizeof(guarantee));
	}
	fputc('\n', stdout);
	i = strlen(prefix);
	char *p = malloc(i + 2);
	strcpy(p, prefix);
	p[i] = ' ';
	p[i+1] = '\0';
	for (i = g->numKids - 1; i >= 0; i--) {
		printGs(p, g->kids[i]);
	}
	free(p);
}
