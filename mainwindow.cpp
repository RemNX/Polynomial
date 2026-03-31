#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    init_graph();
    on_input_input_valueChanged(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}


//empty a layour from their  widgets
void MainWindow::clearLayout(QLayout *layout)
{
    //if layout already empty go next
    if (layout == nullptr)
        return;

    //recursive function
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) //remove first element until there is no more
    {
        if (item->widget())         //if simple widget remove it
            delete item->widget();

        else if (item->layout())    //if layout in layout empty it and remove it
        {
            clearLayout(item->layout());
            delete item->layout();
        }

        delete item;
    }
}

//adapt the number of step to be smooth
double niceStep(double range)
{
    double step = range / 10.0; //get step size for 10 values

    double exponent = std::floor(std::log10(step));     //get the order of magnitude
    double fraction = step / std::pow(10, exponent);    //get 0-10 number of the range

    double niceFraction;
    if (fraction < 1.5)                 //if range really low 1 step = 1
        niceFraction = 1.0;
    else if (fraction < 3.0)            //if range  low 1 step = 2
        niceFraction = 2.0;
    else if (fraction < 7.0)            //if range  high 1 step = 5
        niceFraction = 5.0;
    else                                //if range really high 1 step = 10
        niceFraction = 10.0;

    return niceFraction * std::pow(10, exponent);
}

//event when mouse on graph
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_chartView->viewport() && event->type() == QEvent::MouseMove){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        QPoint pos = mouseEvent->pos();

        // conversion en coordonnées du graphe
        QPointF value = m_chart->mapToValue(pos, m_series);

        double x = value.x();
        double y = value.y();

        ui->point_value_output->setText(QString("x=%1 y=%2").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2));

        // récupérer les limites visibles
        double yMin = m_axisY->min();
        double yMax = m_axisY->max();

        // convertir en coordonnées scène
        QPointF p1 = m_chart->mapToPosition(QPointF(x, yMin), m_series);
        QPointF p2 = m_chart->mapToPosition(QPointF(x, yMax), m_series);

        // mettre à jour la ligne
        m_cursorLine->setLine(QLineF(p1, p2));
    }

    return QObject::eventFilter(obj, event);
}

//create the graph skeleton
void MainWindow::init_graph(){
    m_series = new QLineSeries();           //empty points list
    m_chart  = new QChart();
    m_chart->addSeries(m_series);

    m_axisX = new QValueAxis();             //add x axis
    m_axisX->setTitleText("x");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    m_axisY = new QValueAxis();             //add y axis
    m_axisX->setTitleText("x");
    m_axisY->setTitleText("y");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    m_chartView = new QChartView(m_chart, this);            //render the chart
    m_chartView->setRenderHint(QPainter::Antialiasing);

    //part to get mouse position value
    m_chartView->setMouseTracking(true);
    m_chartView->viewport()->setMouseTracking(true);
    m_chartView->viewport()->installEventFilter(this);

    m_cursorLine = new QGraphicsLineItem();
    m_cursorLine->setPen(QPen(Qt::red, 1, Qt::DashLine));

    m_chart->scene()->addItem(m_cursorLine);

    ui->display_graph->addWidget(m_chartView);              //add the chart to the layout


    //add part for the differents coeff input
    int deg_max = 6;
    QBoxLayout *input_layout = ui->input_layout;
    clearLayout(input_layout);

    for (int i = 0; i < deg_max; i++) {
        QFrame *frame = new QFrame();
        input_list.append(frame);

        frame->setFrameShape(QFrame::Box); //border
        frame->setFrameShadow(QFrame::Raised);

        QVBoxLayout *layout1 = new QVBoxLayout(frame);

        QHBoxLayout *layout2 = new QHBoxLayout();
        QDoubleSpinBox *box = new QDoubleSpinBox();
        QSlider *slider = new QSlider(Qt::Horizontal);

        box->setMinimum(-10000);
        box->setMaximum(10000);
        box->setSingleStep(0.1);

        slider->setMinimum(-100);
        slider->setMaximum(100);

        connect(slider, &QSlider::valueChanged, this, [=](int v) {
            box->setValue(v / 10.0);
        });
        connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged),this, [=](double d) {
            slider->setValue(d * 10);
            refresh_graph();
        });

        //x value
        QLabel *test = new QLabel();
        if (i==deg_max-1){test->setText("x1");}
        else if (i==deg_max-2){test->setText("x");}
        else {test->setText("x^"+QString::number(deg_max-i-1));}

        layout2->addWidget(box,3);
        layout2->addWidget(test,1);

        layout1->addLayout(layout2,1);

        layout1->addWidget(slider,2);

        input_layout->addWidget(frame);
    }
}

//refresh the graph display for new values
void MainWindow::refresh_graph(){
    //get x min max values
    double xmin=ui->input_min->value();
    double xmax=ui->input_max->value();
    int npoints=100;

    //step length
    double step = (xmax-xmin)/(npoints-1);

    //apply the function to every x points
    QList<QPointF> newPoints;
    for (int i = 0; i < npoints; i++){
        double x = xmin + i * step;
        newPoints.append(QPointF(x, polynomial_function(x)));
    }
    m_series->replace(newPoints);

    //get y min and max
    auto [minIt, maxIt] = std::minmax_element(
        newPoints.begin(), newPoints.end(),
        [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); }
        );

    //apply the axis limit    //todo way to change the y axis
    m_axisX->setRange(xmin, xmax);
    //if degree=0 change the y min and max to be seen
    double ymin=minIt->y();
    double ymax=maxIt->y();

    if (ymin==ymax){
        ymin=ymin-5;
        ymax=ymax+5;
    }
    qDebug()<<(ymin==ymax);
    m_axisY->setRange(ymin, ymax);


    //get axis ranges
    double xRange = xmax - xmin;
    double yRange = ymax - ymin;

    //get smooth step
    double xStep = niceStep(xRange);
    double yStep = niceStep(yRange);

    //let the code change the values of axis
    m_axisX->setTickType(QValueAxis::TicksDynamic);
    m_axisY->setTickType(QValueAxis::TicksDynamic);

    //set step interval
    m_axisX->setTickInterval(xStep);
    m_axisY->setTickInterval(yStep);

    //force value on 0
    m_axisX->setTickAnchor(0);
    m_axisY->setTickAnchor(0);


    //part to display the full written function on title
    QList<double> coeffs;
    int degree=ui->input_input->value();
    int max=ui->input_layout->count();
    for (int i = max- degree; i < max; i++){
        QWidget  *container = ui->input_layout->itemAt(i)->widget();
        if (!container) continue;
        QDoubleSpinBox *spinBox   = container->findChild<QDoubleSpinBox*>();
        if (spinBox)
            coeffs.append(spinBox->value());
    }

    ui->Title->setText(polynomialToString(coeffs));
}

//when number of degree changed enable or disable the widgets to change de value
void MainWindow::on_input_input_valueChanged(int value){

    for (int i=0; i<input_list.size();i++){
        input_list[i]->setEnabled((input_list.size()-1-i)<value);     //if i<value = true -> enable
    }
    ui->output_degree->setText(QString::number(value-1));
    refresh_graph();
}

//function calculation
double MainWindow::polynomial_function(double x){
    double res=0;
    int degree= ui->input_input->value();

    //get spinbox values for each coefficients
    QList<double> coeffs;
    int max=ui->input_layout->count();
    for (int i = max- degree; i < max; i++){
        QWidget *container = ui->input_layout->itemAt(i)->widget();
        if (!container) continue;

        QDoubleSpinBox *spinBox = container->findChild<QDoubleSpinBox*>();
        if (spinBox)
            coeffs.append(spinBox->value());
    }

    //function calculation
    for (int i=0;i<coeffs.size();i++){
        res+=coeffs[i]*std::pow(x,coeffs.size()-i-1);
    }
    return res;
}

//transform the function in a string readable as a title
QString MainWindow::polynomialToString(const QList<double> &coeffs)
{
    //if no coefft for now
    if (coeffs.isEmpty()) return "";

    QString result = "f(x) = ";
    int degree = coeffs.size();
    bool firstTerm = true;                      //for the first coeff

    for (int i = 0; i < degree; i++){
        double coeff    = coeffs[i];            //actual coeffficient
        int    exponent = degree - i-1;

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


void MainWindow::on_input_min_valueChanged(double arg1)
{
    refresh_graph();
}


void MainWindow::on_input_max_valueChanged(double arg1)
{
    refresh_graph();
}

