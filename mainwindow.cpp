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

        // coordinate conversion
        QPointF value = m_chart->mapToValue(pos, m_series);

        double x = value.x();
        double y = value.y();

        ui->point_value_output->setText(QString("x=%1 y=%2").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2));

        double yMin = m_axisY->min();
        double yMax = m_axisY->max();

        QPointF p1 = m_chart->mapToPosition(QPointF(x, yMin), m_series);
        QPointF p2 = m_chart->mapToPosition(QPointF(x, yMax), m_series);

        m_cursorLine->setLine(QLineF(p1, p2));
    }

    //if click
    else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            QPointF value = m_chart->mapToValue(mouseEvent->pos(), m_series);
            ui->selected_x->setValue(value.x());

            move_select_point();
            refresh_graph();
        }

    }

    return QObject::eventFilter(obj, event);
}

//create the graph skeleton
void MainWindow::init_graph(){
    //x and x list
    m_series = new QLineSeries();
    m_chart  = new QChart();
    m_series->setName("f(x)");
    m_chart->addSeries(m_series);

    //x selected point
    m_selectedPoint= new QScatterSeries();
    m_selectedPoint->setMarkerSize(10);
    m_chart->addSeries(m_selectedPoint);
    m_chart->legend()->markers(m_selectedPoint)[0]->setVisible(false);

    //derivate first list
    m_seriesfirst = new QLineSeries();
    m_seriesfirst->setName("f'(x)");
    m_chart->addSeries(m_seriesfirst);

    //derivate second list
    m_seriessecond = new QLineSeries();
    m_seriessecond->setName("f''(x)");
    m_chart->addSeries(m_seriessecond);

    //tangent list
    m_seriestangent = new QLineSeries();
    m_seriestangent->setName("tangent");
    m_chart->addSeries(m_seriestangent);

    m_axisX = new QValueAxis();             //add x axis
    m_axisX->setTitleText("x");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_series->attachAxis(m_axisX);
    m_seriesfirst->attachAxis(m_axisX);
    m_selectedPoint->attachAxis(m_axisX);
    m_seriessecond->attachAxis(m_axisX);
    m_seriestangent->attachAxis(m_axisX);

    m_axisY = new QValueAxis();             //add y axis
    m_axisX->setTitleText("x");
    m_axisY->setTitleText("y");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);
    m_selectedPoint->attachAxis(m_axisY);
    m_seriesfirst->attachAxis(m_axisY);
    m_seriessecond->attachAxis(m_axisY);
    m_seriestangent->attachAxis(m_axisY);


    m_series->setColor(Qt::blue);
    m_selectedPoint->setColor(Qt::magenta);
    m_seriesfirst->setColor(Qt::red);
    m_seriessecond->setColor(Qt::green);
    m_seriestangent->setColor(Qt::yellow);

    m_chartView = new QChartView(m_chart, this);            //render the chart
    m_chartView->setRenderHint(QPainter::Antialiasing);

    //part to get mouse position value
    m_chartView->setMouseTracking(true);
    m_chartView->viewport()->setMouseTracking(true);
    m_chartView->viewport()->installEventFilter(this);

    if (m_chart && m_chart->scene()) {
        m_cursorLine = new QGraphicsLineItem();
        m_cursorLine->setPen(QPen(Qt::cyan, 1, Qt::DashLine));
        m_chart->scene()->addItem(m_cursorLine);
    }

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
        QDoubleSpinBox *box = new QDoubleSpinBox(frame);
        QSlider *slider = new QSlider(Qt::Horizontal,frame);

        box->setMinimum(-10000);
        box->setMaximum(10000);
        box->setSingleStep(0.1);
        box->setDecimals(5);

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

//get the coefficients from input
void MainWindow::updateCalculatorCoefficients(){
    int degree = ui->input_input->value();
    int max = ui->input_layout->count();
    QList<double> coeffs;

    for (int i = max - degree; i < max; i++){
        QWidget *container = ui->input_layout->itemAt(i)->widget();
        if (!container) continue;

        QDoubleSpinBox *spinBox = container->findChild<QDoubleSpinBox*>();
        if (spinBox)
            coeffs.append(spinBox->value());
    }

    std::reverse(coeffs.begin(), coeffs.end());

    m_calculator.setCoefficients(coeffs);
}

//refresh the graph display for new values
void MainWindow::refresh_graph(){
    //update coeffs
    updateCalculatorCoefficients();

    move_select_point();

    //get x min max values
    double xmin=ui->input_min->value();
    double xmax=ui->input_max->value();
    int npoints=100;
    double step = (xmax-xmin)/(npoints-1);  //step size


    //apply the function to every x points
    QList<QPointF> newPoints;
    QList<QPointF> newPointsFirst;
    QList<QPointF> newPointsSecond;
    QList<QPointF> newPointsTangent;
    for (int i = 0; i < npoints; i++){
        double x = xmin + i * step;
        newPoints.append(QPointF(x, m_calculator.value(x)));

        //if derivate first or second should be displayed
        if (ui->box_derivate_first->isChecked()){
            newPointsFirst.append(QPointF(x, m_calculator.value(x,1)));
        }
        if (ui->box_derivate_second->isChecked()){
            newPointsSecond.append(QPointF(x, m_calculator.value(x,2)));
        }

        //if tangent should be displayed
        if (ui->box_tangent->isChecked()){
            double a = ui->selected_x->value();
            newPointsTangent.append(QPointF(x, m_calculator.valueTangent(x,a)));
        }
    }
    m_series->replace(newPoints);
    m_seriesfirst->replace(newPointsFirst);
    m_seriessecond->replace(newPointsSecond);
    m_seriestangent->replace(newPointsTangent);

    //get y min and max
    auto [minIt, maxIt] = std::minmax_element(
        newPoints.begin(), newPoints.end(),
        [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); }
    );

    //apply the axis limit
    //if degree=0 change the y min and max to be seen
    double ymin=minIt->y();
    double ymax=maxIt->y();

    if (ymin==ymax){
        ymin=ymin-5;
        ymax=ymax+5;
    }

    m_axisX->setRange(xmin, xmax);
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


    //_______part to display the full written function on title
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
    ui->Title->setText(m_calculator.polynomialToString(coeffs));
    ui->derivate_first_output->setText(m_calculator.polynomialToString(m_calculator.getDerivativeCoefficients(1),1));
    ui->derivate_second_output->setText(m_calculator.polynomialToString(m_calculator.getDerivativeCoefficients(2),2));
}


void MainWindow::move_select_point(){
    double x = ui->selected_x->value();
    double y = m_calculator.value(x);

    m_selectedPoint->clear();
    m_selectedPoint->append(x,y);

    ui->selected_y->setValue(y);
    ui->selected_tangent->setValue(m_calculator.value(x,1));
}


//when number of degree changed enable or disable the widgets to change de value
void MainWindow::on_input_input_valueChanged(int value){

    for (int i=0; i<input_list.size();i++){
        input_list[i]->setEnabled((input_list.size()-1-i)<value);     //if i<value = true -> enable
    }
    ui->output_degree->setText(QString::number(value-1));
    refresh_graph();
}

//input slots  //todo found a way to put them in one function
void MainWindow::on_input_min_valueChanged(double arg1)
{
    refresh_graph();
}


void MainWindow::on_input_max_valueChanged(double arg1)
{
    refresh_graph();
}


void MainWindow::on_box_derivate_first_checkStateChanged(const Qt::CheckState &arg1)
{
    refresh_graph();
}


void MainWindow::on_box_derivate_second_checkStateChanged(const Qt::CheckState &arg1)
{
    refresh_graph();
}


void MainWindow::on_selected_x_valueChanged(double x)
{
    move_select_point();
    refresh_graph();
}


void MainWindow::on_box_tangent_checkStateChanged(const Qt::CheckState &arg1)
{
    refresh_graph();
}

