#include "polynomialcalculator.h"

PolynomialCalculator::PolynomialCalculator() {
}


void PolynomialCalculator::clearCache(){
    m_derivativeCache.clear();      //clear stored derivatives coefficients
}


void PolynomialCalculator::setCoefficients(const QList<double> &coeffs){
    if (coeffs.isEmpty()) {
        qWarning() << "Setting empty coefficient list";
        m_coefficients.clear();
        clearCache();
        return;
    }
    m_coefficients=coeffs;      //store function coefficients
    clearCache();               //new coefficients = new derivative so a clear is needed
}


QList<double> PolynomialCalculator::getCoefficients() const{
    return m_coefficients;
}


QList<double> PolynomialCalculator::getDerivativeCoefficients(int order) const{
    if (order<1){
        qWarning() << "Invalid derivative order:" << order;
        return m_coefficients;
    }
    //check if derivative already in cache
    auto it = m_derivativeCache.find(order);
    if (it != m_derivativeCache.end()) {
        QList<double> res = it.value();
        return res;
    }

    //compute and cache the derivative coefficients
    QList<double> res=DerivativeCoefficient(order);
    m_derivativeCache[order]=res;

    return res;

}


double PolynomialCalculator::value(double x, int derivative_order) const{
    if (m_coefficients.isEmpty()) {
        return 0.0;
    }
    double res=0;

    //select appropriate coefficients
    QList<double> coeffs;
    if (derivative_order==0){               //if usual polynomial
        coeffs=getCoefficients();
    }else if (derivative_order>0){          //if it is a derived polynomial
        coeffs=getDerivativeCoefficients(derivative_order);
    }else {                                 //invalid order
        qDebug()<<"ERROR : Negative order for derivative";
        return 0.0;
    }


    //evaluate polynomial (coeffs[0] = highest degree)
    for (int i=0;i<coeffs.size();i++){
        res+=coeffs[i]*std::pow(x,coeffs.size()-1-i);
    }
    return res;
}


QList<double> PolynomialCalculator::DerivativeCoefficient(int order) const{
    //start from original coefficients
    QList<double> coeffsfirst = getCoefficients();

    //apply derivative for each order
    for (int n=0;n<order;n++){
        QList<double> coeffslast;

        for (int i = 0; i<coeffsfirst.size()-1;i++){
            coeffslast.append(coeffsfirst[i]*(coeffsfirst.size()-1-i));
        }
        coeffsfirst=coeffslast;
    }
    return coeffsfirst;
}


double PolynomialCalculator::valueTangent(double x, double a) const{
    return value(a,1)*(x-a)+value(a);
}


QString PolynomialCalculator::polynomialToString(const QList<double> &coeffs,int order) const{

    //function name f(x), f'(x) or f''(x)
    QString result = "f" + QString("'").repeated(order) + "(x) = ";

    if (coeffs.isEmpty()) return result+"0";

    int degree = coeffs.size();
    bool firstTerm = true;                      //for the first coeff

    for (int i = 0; i < degree; i++){
        double coeff    = coeffs[i];            //current coeffficient
        int    exponent = degree-i-1;

        if (coeff == 0.0) continue;             //do not display coefficients equal to 0

        if (!firstTerm)                             //if first term add a + or a -
            result += (coeff > 0) ? " + " : " - ";
        else if (coeff < 0)                         //else only add - if negative
            result += "-";

        double absCoeff = std::abs(coeff);

        if (absCoeff != 1.0 || exponent == 0)
            result += QString::number(absCoeff, 'g', 4); // 4 decimal number

        if (exponent == 1)              //if it is the second-to-last coefficient
            result += "x";
        else if (exponent > 1)
            result += "x^" + QString::number(exponent);

        firstTerm = false;
    }


    if (firstTerm) return result+"0";
    return result;
}

