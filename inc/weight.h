#define PRE_WEIGHT 	Mmotion = max(0.1, Mmotion); \
					Mentropy = max(5, Mentropy);
#define WEIGHT(face, skin, duration, motion, entropy) weight(face, skin, duration / Mduration, motion / Mmotion, entropy / Mentropy)
inline double weight(double face, double skin, double duration, double motion, double entropy)
{
	if (face > 1)
		face = face = int(face);
	return face * 3 + duration + motion + skin * 0.1 + entropy * 2;
}

#define PCT(v) (int((v)*100+0.5))