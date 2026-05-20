#ifndef POLYNOMIALCALCULATOR_H
#define POLYNOMIALCALCULATOR_H

#include <QString>
#include <QList>
#include <QDebug>
#include <QLineSeries>
#include "types.h"

class PolynomialCalculator
{
public:
    PolynomialCalculator();
    /**
     * @brief change the global coefficient values
     * @param coeffs new coefficient list
     */
    //______coefficients________
    void setCoefficients(const QList<double> &coeffs);
    QList<double> getCoefficients() const;
    QList<double> derivativeCoefficient(int order) const;
    QList<double> getDerivativeCoefficients(int order) const;
    QList<double> getIntegrationCoefficients() const;
    void clearCache();

    //______evaluation________
    double evaluate(double x, int derivative_order=0) const;
    double evaluateTangent(double x, double a) const;
    double evaluateIntegration(double start, double end);

    //______roots & extremas________
    QList<double> approximateX0(QList<QPointF> series);
    double Newton(double x0,int derivative_order);
    QList<double>findAllRoots(QList<double> x0_list,int derivative_order);

    //______text________
    QString polynomialToString(const QList<double> &coeffs,int order=0) const;

private:
    QList<double> m_coefficients;
    mutable QHash<int, QList<double>>  m_derivativeCache;
};

#endif // POLYNOMIALCALCULATOR_H
