
#include <stdint.h>
#include <stdio.h>
#include <math.h>      // atan()

// Number of bits for Cordic units.
#define NBITS       14

// 360 degrees = 65536 Cordic angle units (cau).
#define CAU_BASE    65536

// Quadrants for angles in Cordic calculations.
#define QUAD1   1
#define QUAD2   2
#define QUAD3   3
#define QUAD4   4

#define M_PI    3.14159265358979323846

typedef unsigned int WORD;

void SinCosSetup(void);
void SinCos(WORD theta, int *sine, int *cosine);

int  ArcTan[NBITS];      // angles for Cordic rotation
int  xInit;              // initial x projection

WORD CordicBase;         // base for Cordic units
WORD HalfBase;           // CordicBase / 2
WORD Quad2Boundary;      // 2 * CordicBase
WORD Quad3Boundary;      // 3 * CordicBase

/*
 *  Output should be:
 *  ------------------------------------------
 *  theta_cau = 1820, sin =  2847, cos = 16134
 *  theta_cau = 3641, sin =  5605, cos = 15393
 *  theta_cau = 5461, sin =  8191, cos = 14191
 *  theta_cau = 7282, sin = 10531, cos = 12551
 *  ------------------------------------------
 *
 *  These angles are 10 degrees, 20 degrees, 30 degrees, and 40 degrees.
 *   For the first line, 10 degrees = (10 / 360) * 65536 ~ 1820 cau
 *   sin(10 deg) = 0.1736 and 0.1736 * 16384 =  2845.05 ~  2847 cru
 *   cos(10 deg) = 0.9848 and 0.9848 * 16384 = 16135.09 ~ 16134 cru
 */
int main(int argc, char* argv[])
{
    int cosine = -2;
    int sine = -2;
    int theta_degrees;
    int theta_cau;

    SinCosSetup();

    // 10 degrees, 20 degrees, 30 degrees, and 40 degrees.
    for (theta_degrees = 10; theta_degrees < 50; theta_degrees += 10)
    {
        theta_cau = (theta_degrees * CAU_BASE + 180) / 360;
        SinCos(theta_cau, &sine, &cosine);
        printf("theta_cau = %d, sin = %5d, cos = %5d\n", theta_cau, sine, cosine);
    }

    return 0;
}

/**
 *  USE:  Load globals used by SinCos().
 *  OUT:  Loads globals used in SinCos() :
 *          CordicBase    = base for CORDIC units
 *          HalfBase      = Cordicbase / 2
 *          Quad2Boundary = 2 * CordicBase
 *          Quad3Boundary = 3 * CordicBase
 *          ArcTan[]      = the arctans of 1/(2^i)
 *          xInit         = initial value for x projection
 *  NOTE: Must be called once only to initialize before calling SinCos().
 *        xInit is less than CordicBase exactly the amount required to
 *        compensate for the accumulated expansions in the rotations.
 */
void SinCosSetup(void)
{
    int i;        // to index ArcTan[]
    double f;     // to calc initial x projection
    long powr;    // powers of 2 up to 2^(2*(NBITS-1))

    CordicBase = 1 << NBITS;
    HalfBase = CordicBase >> 1;
    Quad2Boundary = CordicBase << 1;
    Quad3Boundary = CordicBase + Quad2Boundary;

    // ArcTan's are diminishingly small angles.
    powr = 1;
    for (i = 0; i < NBITS; i++)
    {
        ArcTan[i] = (int) (atan(1.0/powr)/(M_PI/2)*CordicBase + 0.5);
        powr <<= 1;
    }

    #if 1
    printf("\natan table: \n");
    for (i = 0; i < NBITS; i++)
    {
        if( (i & 0x7) == 0 )
            printf("\n");

        printf("%4d, ", ArcTan[i]);
    }
    printf("\n");
    #endif // 1

    /* xInit is initial value of x projection to comp-
     * ensate for expansions.  f = 1/sqrt(2/1 * 5/4 * ...
     * Normalize as an NBITS binary fraction (multiply by
     * CordicBase) and store in xInit. Get f = 0.607253
     * and xInit = 9949 = 0x26DD for NBITS = 14.
     */
    f = 1.0;
    powr = 1;
    for (i = 0; i < NBITS; i++)
    {
        f = (f * (powr + 1)) / powr;
        powr <<= 2;
    }
    f = 1.0/sqrt(f);
    xInit = (int) (CordicBase * f + 0.5);
}

/**
 *  USE:  Calc sine and cosine with integer CORDIC routine.
 *  IN :  theta = incoming angle (in CORDIC angle units)
 *  OUT:  sine = ptr to sine (in CORDIC fixed point units)
 *        cosine = ptr to cosine (in CORDIC fixed point units)
 *  NOTE: The incoming angle theta is in CORDIC angle
 *        units, which subdivide the circle into 64K parts,
 *        with 0 deg = 0, 90 deg = 16384 (CordicBase), 180 deg
 *        = 32768, 270 deg = 49152, etc. The calculated sine
 *        and cosine are in CORDIC fixed point units : an int
 *        considered as a fraction of 16384 (CordicBase).
 */
void SinCos(WORD theta, int *sine, int *cosine)
{
    int quadrant;  // quadrant of incoming angle
    int z;         // incoming angle moved to 1st quad
    int i;         // to index rotations : one per bit
    int x, y;      // projections onto axes
    int x1, y1;    // projections of rotated vector

    /* Determine quadrant of incoming angle, translate to
     * 1st quadrant. Note use of previously calculated
     * values CordicBase, etc. for speed.
     */
    if (theta < CordicBase)
    {
        quadrant = QUAD1;
        z = (int) theta;
    }
    else if (theta < Quad2Boundary)
    {
        quadrant = QUAD2;
        z = (int) (Quad2Boundary - theta);
    }
    else if (theta < Quad3Boundary)
    {
        quadrant = QUAD3;
        z = (int) (theta - Quad2Boundary);
    }
    else
    {
        quadrant = QUAD4;
        // Line below works with 16-bit ints but fails if ints are larger!
        //z = - ((int) theta);
        z = CAU_BASE - theta;
    }

    // Initialize projections.
    x = xInit;
    y = 0;

    /* Negate z, so same rotations taking angle z to 0
     * will take (x, y) = (xInit, 0) to (*cosine, *sine).
     */
    z = -z;

    // Rotate NBITS times.
    for (i = 0; i < NBITS; i++)
    {
        if (z < 0)
        {
            // Counter-clockwise rotation.
            z += ArcTan[i];
            y1 = y + (x >> i);
            x1 = x - (y >> i);
        }
        else
        {
            // Clockwise rotation.
            z -= ArcTan[i];
            y1 = y - (x >> i);
            x1 = x + (y >> i);
        }

        // Put new projections into (x,y) for next go.
        x = x1;
        y = y1;
    }  /* for i */

    // Attach signs depending on quadrant.
    *cosine = (quadrant==QUAD1 || quadrant==QUAD4) ? x : -x;
    *sine   = (quadrant==QUAD1 || quadrant==QUAD2) ? y : -y;
}
