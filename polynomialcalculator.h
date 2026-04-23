#ifndef POLYNOMIALCALCULATOR_H
#define POLYNOMIALCALCULATOR_H

#include <QString>
#include <QList>
#include <QDebug>
#include <QLineSeries>

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

    /**
     * @brief get the cofficients value for the like ax^n -> (a/n+1)x^(n+1)
     */
    QList<double> GetIntegrationCoefficients() const;

    /**
     * @brief get the integration value from a range of the polynomial function
     * @param start point of the integration
     * @param end point of the integration
     * @return a double value of the integration
     */
    double IntegrationValue(double start, double end);

    /**
     * @brief Approximate the x value of a series where x=0, only use the points of the serie so more point = more precision but never perfect
     * @param series of x,y point
     * @return a list of all the x0 point (some can be duplicates)
     */
    QList<double> ApproximateX0(QLineSeries *series);

    /**
     * @brief calculate the y=0 (root) value of an x0 by the newton method
     * @param x0 point close to y=0 can be determined with the approximateX0(series) function
     * @return a really precise value of the root
     */
    double RootNewton(double x0);

    /**
     * @brief search all roots value of a list of x0 (remove the duplicate and approximate x=0
     * @param x0_list can be obtained with the function ApproximateX0(series)
     * @return a list of all roots value of the polynomial function
     */
    QList<double>AllRoot(QList<double> x0_list);

private:
    static constexpr double ROOT_PRECISION=1e-10;
    static constexpr int MAX_ROOT_LOOP=50;
    QList<double> m_coefficients;
    mutable QHash<int, QList<double>>  m_derivativeCache;
};

#endif // POLYNOMIALCALCULATOR_H
