#include "polynomialcalculator.h"

PolynomialCalculator::PolynomialCalculator() {
}

void PolynomialCalculator::clearCache(){
    m_derivativeCache.clear();
}

void PolynomialCalculator::setCoefficients(const QList<double> &coeffs){
    m_coefficients =coeffs;
    clearCache(); //new coefficients = new derivative
}

QList<double> PolynomialCalculator::getCoefficients() const{
    return m_coefficients;
}

QList<double> PolynomialCalculator::getDerivativeCoefficients(int order) const{
    if (order<1){
        qDebug()<<"ERROR : Order for derivate coefficients under 1";
    }
    //cache verification
    if (m_derivativeCache.contains(order)){
        return m_derivativeCache[order];
    }

    QList<double> res=DerivativeCoefficient(order);
    m_derivativeCache[order]=res;

    return res;

}

double PolynomialCalculator::value(double x, int derivative_order) const{
    if (m_coefficients.isEmpty()) {
        return 0.0;
    }
    double res=0;

    //get coeffs
    QList<double> coeffs;
    if (derivative_order==0){               //if usual polynomial
        coeffs=getCoefficients();
    }else if (derivative_order>0){          //if derivated polynomial
        coeffs=getDerivativeCoefficients(derivative_order);
    }else {                                 //wrong order
        qDebug()<<"ERROR : Negative order for derivative";
        return 0.0;
    }

    //function calculation
    for (int i=0;i<coeffs.size();i++){
        res+=coeffs[i]*std::pow(x,i);
    }
    return res;
}

QList<double> PolynomialCalculator::DerivativeCoefficient(int order) const{
    //derivate coefficients
    QList<double> coeffsfirst = getCoefficients();

    for (int n=0;n<order;n++){
        QList<double> coeffslast;

        for (int i = 1; i<coeffsfirst.size();i++){
            coeffslast.append(coeffsfirst[i]*i);
        }
        coeffsfirst=coeffslast;
    }
    return coeffsfirst;
}

double PolynomialCalculator::valueTangent(double x, double a) const{
    return value(a,1)*(x-a)+value(a);
}

QString PolynomialCalculator::polynomialToString(const QList<double> &coeffs,int order){
    //if no coeffs for now
    if (coeffs.isEmpty()) return "";

    QString result = "f" + QString("'").repeated(order) + "(x) = ";;
    int degree = coeffs.size();
    bool firstTerm = true;                      //for the first coeff

    for (int i = degree-1; i >= 0; i--){
        double coeff    = coeffs[i];            //actual coeffficient
        int    exponent = i;

        if (coeff == 0.0) continue;             //dont display coeff if 0

        if (!firstTerm)                             //if first term add a + or a -
            result += (coeff > 0) ? " + " : " - ";
        else if (coeff < 0)                         //else only add - if negative
            result += "-";

        double absCoeff = std::abs(coeff);

        if (absCoeff != 1.0 || exponent == 0)
            result += QString::number(absCoeff, 'g', 4); // 4 decimal number

        if (exponent == 1)              //if pre last coeff
            result += "x";
        else if (exponent > 1)
            result += "x^" + QString::number(exponent);

        firstTerm = false;
    }

    if (firstTerm) return "f(x) = 0";
    return result;
}

