#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    //init chart and UI
    initGraph();


    connect(ui->box_derivate_first, &QCheckBox::checkStateChanged, this, &MainWindow::refreshGraph);
    connect(ui->box_derivate_second, &QCheckBox::checkStateChanged,  this, &MainWindow::refreshGraph);
    connect(ui->box_tangent, &QCheckBox::checkStateChanged, this, &MainWindow::refreshGraph);
    connect(ui->visible_integration, &QCheckBox::checkStateChanged,this, &MainWindow::refreshAxes);

    on_input_input_valueChanged(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearLayout(QLayout *layout)
{
    //if the layout is already empty, return
    if (layout == nullptr)
        return;

    //recursive call
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) //remove the first element until none remain
    {
        if (item->widget())         //if it is a simple widget, remove it
            item->widget()->deleteLater();

        else if (item->layout())    //if it is a nested layout, clear and remove it
        {
            clearLayout(item->layout());
            delete item->layout();
        }

        delete item;
    }
}

/**
 * @brief Compute a "nice" step size for axis ticks
 *
 * The function returns a rounded step value (1, 2, 5, or 10 × power of 10)
 * to make axis labels more readable.
 *
 * @param range The total range (max - min)
 * @return readable step size
 */
double niceStep(double range)
{
    double step = range / 10.0; //get step size for 10 values

    double exponent = std::floor(std::log10(step));     //get the order of magnitude
    double fraction = step / std::pow(10, exponent);    //get a value between 0 and 10

    double niceFraction;
    if (fraction < 1.5)                 //if the range is very small 1 step = 1
        niceFraction = 1.0;
    else if (fraction < 3.0)            //if range is low 1 step = 2
        niceFraction = 2.0;
    else if (fraction < 7.0)            //if range is large 1 step = 5
        niceFraction = 5.0;
    else                                //if range is very large 1 step = 10
        niceFraction = 10.0;

    return niceFraction * std::pow(10, exponent);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    //handle mouse movement
    if (obj == m_chartView->viewport() && event->type() == QEvent::MouseMove){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        QPoint pos = mouseEvent->pos();

        //convert coordinates
        QPointF value = m_chart->mapToValue(pos, m_series);

        double x = value.x();
        double y = value.y();

        //display current coordinate values
        ui->point_value_output->setText(QString("x=%1 y=%2").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2));

        double yMin = m_axisY->min();
        double yMax = m_axisY->max();

        //draw a vertical line
        QPointF p1 = m_chart->mapToPosition(QPointF(x, yMin), m_series);
        QPointF p2 = m_chart->mapToPosition(QPointF(x, yMax), m_series);
        m_cursorLine->setLine(QLineF(p1, p2));
    }

    //handle mouse click
    else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            QPointF value = m_chart->mapToValue(mouseEvent->pos(), m_series);
            if (ui->startendbutton->isChecked()){       //if integration button checked
                if (m_integration_point_counter==0) ui->start_integration_input->setValue(value.x());
                if (m_integration_point_counter==1) ui->end_integration_input->setValue(value.x());

                m_integration_point_counter+=1;
                if (m_integration_point_counter>1) {
                    ui->startendbutton->setChecked(false);
                    m_integration_point_counter=0;
                }

                refreshEquations();
                refreshAxes();
            }else{                                      //usual click
                //update selected x and recompute
                ui->selected_x->setValue(value.x());
            }


        }

    }

    return QObject::eventFilter(obj, event);
}

void MainWindow::initGraph(){
    //x and y series
    m_series = new QLineSeries(this);
    m_chart  = new QChart();

    //y=0 line should be first to be under all the other series
    m_zeroSeries = new QLineSeries();
    m_zeroSeries->setColor(Qt::black);
    m_chart->addSeries(m_zeroSeries);

    // area between curve and y=0
    m_integrationBaseSeries = new QLineSeries();
    m_integrationBaseSeries->setVisible(false);
    m_chart->addSeries(m_integrationBaseSeries);

    m_integrationSeries = new QLineSeries(this);
    m_chart->addSeries(m_integrationSeries);

    m_areaSeries = new QAreaSeries(m_integrationSeries, m_integrationBaseSeries);


    QPen pen(Qt::NoPen);
    m_areaSeries->setPen(pen);
    m_areaSeries->setBrush(QColor(0, 0, 255, 50));

    m_chart->addSeries(m_areaSeries);

    m_series->setName("f(x)");
    m_chart->addSeries(m_series);

    //first derivative series
    m_seriesfirst = new QLineSeries(this);
    m_seriesfirst->setName("f'(x)");
    m_chart->addSeries(m_seriesfirst);

    //second derivative series
    m_seriessecond = new QLineSeries(this);
    m_seriessecond->setName("f''(x)");
    m_chart->addSeries(m_seriessecond);

    //tangent series
    m_seriestangent = new QLineSeries(this);
    m_seriestangent->setName("tangent");
    m_chart->addSeries(m_seriestangent);

    //selected x point
    m_selectedPoint= new QScatterSeries();
    m_selectedPoint->setMarkerSize(10);
    m_chart->addSeries(m_selectedPoint);
    m_chart->legend()->markers(m_selectedPoint)[0]->setVisible(false);

    m_axisX = new QValueAxis();             //add x axis
    m_axisX->setTitleText("x");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_zeroSeries->attachAxis(m_axisX);
    m_areaSeries->attachAxis(m_axisX);
    m_series->attachAxis(m_axisX);
    m_seriesfirst->attachAxis(m_axisX);
    m_selectedPoint->attachAxis(m_axisX);
    m_seriessecond->attachAxis(m_axisX);
    m_seriestangent->attachAxis(m_axisX);

    m_axisY = new QValueAxis();             //add y axis
    m_axisX->setTitleText("x");
    m_axisY->setTitleText("y");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_zeroSeries->attachAxis(m_axisY);
    m_areaSeries->attachAxis(m_axisY);
    m_series->attachAxis(m_axisY);
    m_selectedPoint->attachAxis(m_axisY);
    m_seriesfirst->attachAxis(m_axisY);
    m_seriessecond->attachAxis(m_axisY);
    m_seriestangent->attachAxis(m_axisY);

    m_chart->legend()->markers(m_zeroSeries)[0]->setVisible(false);
    m_chart->legend()->markers(m_integrationBaseSeries)[0]->setVisible(false);
    m_chart->legend()->markers(m_integrationSeries)[0]->setVisible(false);
    m_chart->legend()->markers(m_areaSeries)[0]->setVisible(false);


    m_zeroSeries->setColor(Qt::black);
    m_series->setColor(Qt::blue);
    m_selectedPoint->setColor(Qt::magenta);
    m_seriesfirst->setColor(Qt::red);
    m_seriessecond->setColor(Qt::green);
    m_seriestangent->setColor(Qt::yellow);


    m_chartView = new QChartView(m_chart, this);            //render the chart
    m_chartView->setRenderHint(QPainter::Antialiasing);

    if (m_chart && m_chart->scene()) {
        m_cursorLine = new QGraphicsLineItem();
        m_cursorLine->setPen(QPen(Qt::cyan, 1, Qt::DashLine));
        m_chart->scene()->addItem(m_cursorLine);

        //add integration delimitation line
        m_startIntegrationLine = new QGraphicsLineItem();
        m_startIntegrationLine->setPen(QPen(Qt::black, 1));
        m_startIntegrationLine->setVisible(false);
        m_chart->scene()->addItem(m_startIntegrationLine);

        m_endIntegrationLine = new QGraphicsLineItem();
        m_endIntegrationLine->setPen(QPen(Qt::black, 1));
        m_endIntegrationLine->setVisible(false);
        m_chart->scene()->addItem(m_endIntegrationLine);
    }

    //part to get mouse position value
    m_chartView->setMouseTracking(true);
    m_chartView->viewport()->setMouseTracking(true);
    m_chartView->viewport()->installEventFilter(this);

    ui->display_graph->addWidget(m_chartView);              //add the chart to the layout


    //add the coefficient input section
    QBoxLayout *input_layout = ui->input_layout;
    clearLayout(input_layout);

    for (int i = 0; i < MAX_DEGREE; i++) {
        QFrame *frame = new QFrame(this);
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
            refreshGraph();
        });

        //x value
        QLabel *test = new QLabel();
        if (i==MAX_DEGREE-1){test->setText("x1");}
        else if (i==MAX_DEGREE-2){test->setText("x");}
        else {test->setText("x^"+QString::number(MAX_DEGREE-i-1));}

        layout2->addWidget(box,3);
        layout2->addWidget(test,1);

        layout1->addLayout(layout2,1);

        layout1->addWidget(slider,2);

        input_layout->addWidget(frame);
    }
}

void MainWindow::refreshGraph(){
    //Block signals to prevent an infinite loop
    ui->selected_y->blockSignals(true);
    ui->selected_tangent->blockSignals(true);

    //update coeffs
    updateCalculatorCoefficients();

    move_select_point();

    //all updates
    refreshSeries();
    refreshAxes();
    refreshEquations();

    ui->selected_y->blockSignals(false);
    ui->selected_tangent->blockSignals(false);
}

void MainWindow::refreshSeries(){
    //get x min and max values
    double xmin = ui->input_min->value();
    double xmax = ui->input_max->value();
    double step = (xmax - xmin) / (NUMBER_POINTS - 1);

    QList<QPointF> newPoints, newPointsFirst, newPointsSecond, newPointsTangent;
    newPoints.reserve(NUMBER_POINTS);

    //apply the function to every x point
    for (int i = 0; i < NUMBER_POINTS; i++) {
        double x = xmin + i * step;
        newPoints.append(QPointF(x, m_calculator.value(x)));

        //if the first or second derivative should be displayed
        if (ui->box_derivate_first->isChecked()) {
            newPointsFirst.append(QPointF(x, m_calculator.value(x, 1)));
        }
        if (ui->box_derivate_second->isChecked()) {
            newPointsSecond.append(QPointF(x, m_calculator.value(x, 2)));
        }
        //if the tangent should be displayed
        if (ui->box_tangent->isChecked()) {
            double a = ui->selected_x->value();
            newPointsTangent.append(QPointF(x, m_calculator.valueTangent(x, a)));
        }
    }

    m_series->replace(newPoints);
    m_seriesfirst->replace(newPointsFirst);
    m_seriessecond->replace(newPointsSecond);
    m_seriestangent->replace(newPointsTangent);
}

void MainWindow::refreshAxes(){
    double xmin = ui->input_min->value();
    double xmax = ui->input_max->value();

    QVector<QPointF> newPoints = m_series->points();

    if (newPoints.isEmpty()){
        return;
    }

    //get y min and max values
    auto [minIt, maxIt] = std::minmax_element(
        newPoints.begin(), newPoints.end(),
        [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); }
        );


    //apply the axis limit
    //if the degree is 0, adjust y min and max for visibility
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

    //get a smooth step
    double xStep = niceStep(xRange);
    double yStep = niceStep(yRange);

    //let the code adjust the axis values
    m_axisX->setTickType(QValueAxis::TicksDynamic);
    m_axisY->setTickType(QValueAxis::TicksDynamic);

    //set step interval
    m_axisX->setTickInterval(xStep);
    m_axisY->setTickInterval(yStep);

    //force the axis to include 0
    m_axisX->setTickAnchor(0);
    m_axisY->setTickAnchor(0);

    //y=0 line
    QList<QPointF> zeroPoints;
    zeroPoints.append(QPointF(xmin, 0));
    zeroPoints.append(QPointF(xmax, 0));

    m_zeroSeries->replace(zeroPoints);
    m_zeroSeries->setVisible(ymin <= 0 && ymax >= 0);

    //integration display
    if (ui->visible_integration->isChecked()){
        double startx=ui->start_integration_input->value();
        double endx=ui->end_integration_input->value();
        QPointF start1 = m_chart->mapToPosition(QPointF(startx, ymin), m_series);
        QPointF start2 = m_chart->mapToPosition(QPointF(startx, ymax), m_series);
        QPointF end1 = m_chart->mapToPosition(QPointF(endx, ymin), m_series);
        QPointF end2 = m_chart->mapToPosition(QPointF(endx, ymax), m_series);
        m_startIntegrationLine->setLine(QLineF(start1, start2));
        m_startIntegrationLine->setVisible(true);
        m_endIntegrationLine->setLine(QLineF(end1, end2));
        m_endIntegrationLine->setVisible(true);

        updateIntegrationArea();
        m_areaSeries->setVisible(true);
    }else {
        m_startIntegrationLine->setVisible(false);
        m_endIntegrationLine->setVisible(false);

        m_areaSeries->setVisible(false);
    }
}

void MainWindow::refreshEquations(){
    //display the full function in the title and in the derivative part
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
    ui->integration_equation_output->setText(m_calculator.polynomialToString(m_calculator.GetIntegrationCoefficients(),-1));
    ui->integrationvalue_output->setValue(m_calculator.IntegrationValue(ui->start_integration_input->value(),ui->end_integration_input->value()));
}

void MainWindow::move_select_point(){
    double x = ui->selected_x->value();
    double y = m_calculator.value(x);

    m_selectedPoint->clear();
    m_selectedPoint->append(x,y);

    ui->selected_y->setValue(y);
    ui->selected_tangent->setValue(m_calculator.value(x,1));
    ui->selected_der1->setValue(m_calculator.value(x,1));
    ui->selected_der2->setValue(m_calculator.value(x,2));
}

void MainWindow::updateCalculatorCoefficients(){
    int degree = ui->input_input->value();
    int max = ui->input_layout->count();
    QList<double> coeffs;

    for (int i = max - degree; i < max; i++){
        if (auto *container = qobject_cast<QWidget*>(ui->input_layout->itemAt(i)->widget())) {
            if (auto *spinBox = container->findChild<QDoubleSpinBox*>()) {
                coeffs.append(spinBox->value());
            }
        }
    }

    m_calculator.setCoefficients(coeffs);
}

void MainWindow::updateIntegrationArea(){
    double start = ui->start_integration_input->value();
    double end   = ui->end_integration_input->value();

    if (start > end)
        std::swap(start, end);

    QVector<QPointF> top;
    QVector<QPointF> bottom;

    int N = NUMBER_POINTS;
    double step = (end - start) / (N - 1);

    for (int i = 0; i < N; i++) {
        double x = start + i * step;
        double y = m_calculator.value(x);

        top.append(QPointF(x,y));
        bottom.append(QPointF(x,0));

    }

    m_integrationSeries->replace(top);
    m_integrationBaseSeries->replace(bottom);
}

//input slots
void MainWindow::on_input_input_valueChanged(int value){

    for (int i=0; i<input_list.size();i++){
        input_list[i]->setEnabled((input_list.size()-1-i)<value);     //if i<value = true -> enable
    }
    ui->output_degree->setText(QString::number(value-1));
    refreshGraph();
}

void MainWindow::on_selected_x_valueChanged()
{
    move_select_point();
    refreshGraph();
}

void MainWindow::on_input_min_valueChanged(double value) {
    if (value >= ui->input_max->value()) {
        ui->input_max->setValue(value+1);
    }
    refreshGraph();
}

void MainWindow::on_input_max_valueChanged(double value) {
    if (value <= ui->input_min->value()) {
        ui->input_min->setValue(value-1);
    }
    refreshGraph();
}

