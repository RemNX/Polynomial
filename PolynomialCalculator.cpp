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
    }else if (derivative_order==-1){         //if its in the integration
        coeffs=GetIntegrationCoefficients();
    }else{                                 //invalid order
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

    //function name f(x), f'(x) or f''(x) or F(x)
    QString result;
    if (order<0){
        result = "F(x) = ";
    }else {
        result = "f" + QString("'").repeated(order) + "(x) = ";
    }


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

    QString LastTerm;
    if (order<0){
        if (firstTerm) LastTerm = "C";
        else LastTerm = " + C";
    }else{
        if (firstTerm) LastTerm = "0";
    }

    return result+LastTerm;
}


QList<double> PolynomialCalculator::GetIntegrationCoefficients() const{
    QList <double> coeffs=m_coefficients;
    if (coeffs.isEmpty()) {
        return {0};
    }

    int degree=coeffs.size();
    QList<double> newcoeffs;

    for (int i=0;i<degree;i++){
        double val=coeffs[i]/(degree-i);
        newcoeffs.append(val);
    }
    newcoeffs.append(0);

    return newcoeffs;
}

double PolynomialCalculator::IntegrationValue(double start, double end){
    double start_value = value(start,-1);
    double end_value = value(end,-1);

    return end_value-start_value;
}

QList<double> PolynomialCalculator::ApproximateX0(QLineSeries *series){
    QList<double> coeffs = getCoefficients();
    bool rootable = false;
    for (int i=0; i<coeffs.size()-1;i++){       //if there is only zero for all coefficient except the constant one there is not root value
        if (coeffs[i]!=0){
            rootable=true;
            break;
        }
    }
    if (!rootable){
        return QList<double>();
    }

    QList<double> res;
    QList<QPointF> pts = series->points();
    for (int i=1;i<pts.size();i++) {
        QPointF point1=pts[i-1];
        QPointF point2=pts[i];

        double eps = 1e-5;

        //if change of sign -> there is an y=0
        if (point1.y() * point2.y() < 0.0) {
            double x0 = point1.x() - point2.y() * (point2.x() - point1.x()) / (point2.y() - point1.y()); // interpolation linéaire
            res.append(x0);
        }

        //if a point is really close to zero -> if a boundarie is close to the root
        else if (fabs(point1.y()) < eps) {
            res.append(point1.x());
        }

        //if a point is really close to zero but his neighboor are not -> a local extremum
        if (i > 0 && i < pts.size() - 1) {
            double y_prev = pts[i-1].y();
            double y_curr = pts[i].y();
            double y_next = pts[i+1].y();

            if (fabs(y_curr) < eps &&
                fabs(y_curr) < fabs(y_prev) &&
                fabs(y_curr) < fabs(y_next)) {

                res.append(pts[i].x());
            }
        }
    }
    return res;
}

double PolynomialCalculator::RootNewton(double x0){
    double x = x0;
    for (int loop=0;loop<MAX_ROOT_LOOP;loop++){
        double fx = value(x);
        double dfx = value(x,1);

        if (fabs(dfx) < 1e-12) {
            x += 1e-6;
            continue;
        }

        double xplus1=x-fx/dfx;
        if (fabs(xplus1-x)<ROOT_PRECISION){
            return xplus1;}

        x=xplus1;
    }
    return x;
}

QList<double> PolynomialCalculator::AllRoot(QList<double> x0_list){
    QList<double> res;
    for (double x0 : x0_list){

        double newton_x0 = RootNewton(x0);

        //if close to zero
        if (fabs(newton_x0) <1e-8){
            newton_x0=0;
        }

        //check if this root already in the list
        bool exist=false;
        for (double val : res){     //todo use this format for other for loop !!
            if (fabs(val-newton_x0)<1e-5){
                exist=true;
                break;
            }
        }
        if (!exist) res.append(newton_x0);
    }
    return res;
}
