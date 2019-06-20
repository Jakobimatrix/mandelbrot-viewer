#ifndef RAND_GENERATORS_H
#define RAND_GENERATORS_H

#include <random>
#include <ctime>
#include <limits>

namespace func {

// \brief uniform_int_dist(int min, int max);
// Rückgabe:Integer Zufallsvariable einer Gleichverteilung von min und max

//[min]	Anfang der Gleichverteilung
//[max] Ende der Gleichverteilung
int uniform_int_dist(int min, int max);

// \brief randTrueFalse();
// Rückgabe: zufällig true oder false (50/50)
bool randTrueFalse();

// \brief uniform_double_dist(double max, double min);
// Rückgabe:double Zufallsvariable einer Gleichverteilung von min und max

//[min]	Anfang der Gleichverteilung
//[max] Ende der Gleichverteilung
double uniform_double_dist(double max, double min);

// \brief randGaus(double mean, double variance);
// Rückgabe:double Zufallsvariable einer Normalverteilung N(mean,variance^2)

//[mean]		Erwartungswert der Normalverteilten Zufallsvariable
//[variance]	Varianz der Normalverteilten Zufallsvariable
double randGaus(double mean, double variance);


}// namespace func


#endif
