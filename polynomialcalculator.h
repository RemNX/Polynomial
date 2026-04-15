#ifndef POLYNOMIALCALCULATOR_H
#define POLYNOMIALCALCULATOR_H

#include <QString>
#include <QList>
#include <QDebug>

class PolynomialCalculator
{
public:
    PolynomialCalculator();
    /**
     * @brief change the global coefficient values
     * @param coeffs new coefficient list
     */
    void setCoefficients(const QList<double> &coeffs);

    /**
     * @brief get coefficients from the global variable
     */
    QList<double> getCoefficients() const;

    /**
     * @brief check if coefficients exist in the cache, otherwise compute them
     * @param order of the derivative
     * @return list of coefficients
     */
    QList<double> getDerivativeCoefficients(int order) const;

    //calculation
    /**
     * @brief Evaluate the polynomial or its derivative at a given x
     * @param x point
     * @param derivative_order:
     *        0 → polynomial
     *        n > 0 → nth derivative
     * @return
     */
    double value(double x, int derivative_order=0) const;

    /**
     * @brief get derivative coefficients from the current coefficients
     * @param order of derivative
     * @return a list of coefficients like the input one
     */
    QList<double> DerivativeCoefficient(int order) const;

    /**
     * @brief get the value of a point on the tangent line at a given point
     * @param x point of the polynomial equation
     * @param a at which point the tangent is supposed to be calculated
     * @return a y value
     */
    double valueTangent(double x, double a) const;

    /**
     * @brief transform a list of coefficients into a string to display the correct equation
     * @param coeffs: list of coefficients
     * @param order: order of derivative to determine the number of '
     * @return a QString of the equation
     */
    QString polynomialToString(const QList<double> &coeffs,int order=0) const;

    /**
     * @brief clear the coefficient cache before updating values
     */
    void clearCache();

private:
    QList<double> m_coefficients;
    mutable QHash<int, QList<double>>  m_derivativeCache;
};

#endif // POLYNOMIALCALCULATOR_H
