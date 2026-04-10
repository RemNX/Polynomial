#ifndef POLYNOMIALCALCULATOR_H
#define POLYNOMIALCALCULATOR_H

#include <QString>
#include <QList>
#include <QDebug>

class PolynomialCalculator
{
public:
    PolynomialCalculator();

    void setCoefficients(const QList<double> &coeffs);
    QList<double> getCoefficients() const;
    QList<double> getDerivativeCoefficients(int order) const;

    //calculus
    double value(double x, int derivative_order=0) const;
    QList<double> DerivativeCoefficient(int order) const;
    double valueTangent(double x, double a) const;
    QString polynomialToString(const QList<double> &coeffs,int order=0);

    void clearCache();

private:
    QList<double> m_coefficients;
    mutable QMap<int, QList<double>> m_derivativeCache;
};

#endif // POLYNOMIALCALCULATOR_H
