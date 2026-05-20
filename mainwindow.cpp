#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_chartView(nullptr)
    , m_chart(nullptr)
    , m_axisX(nullptr)
    , m_axisY(nullptr)
    , m_series(nullptr)
    , m_seriesfirst(nullptr)
    , m_seriessecond(nullptr)
    , m_seriestangent(nullptr)
    , m_selectedPoint(nullptr)
    , m_cursorLine(nullptr)
    , m_zeroSeries(nullptr)
    , m_startIntegrationLine(nullptr)
    , m_endIntegrationLine(nullptr)
    , m_integrationBaseSeries(nullptr)
    , m_integrationSeries(nullptr)
    , m_areaSeries(nullptr)
    , m_zoomMode(false)
    , m_zoomClickCount(0)
{
    ui->setupUi(this);
    //init chart and UI
    initGraph();

    connect(ui->box_derivate_first, &QCheckBox::checkStateChanged, this, &MainWindow::refreshGraph);
    connect(ui->box_derivate_second, &QCheckBox::checkStateChanged,  this, &MainWindow::refreshGraph);
    connect(ui->box_tangent, &QCheckBox::checkStateChanged, this, &MainWindow::refreshGraph);
    connect(ui->visible_integration, &QCheckBox::checkStateChanged,this, &MainWindow::refreshAxes);
    connect(ui->start_integration_input, &QDoubleSpinBox::valueChanged, this, &MainWindow::refreshAxes);
    connect(ui->end_integration_input, &QDoubleSpinBox::valueChanged,this, &MainWindow::refreshAxes);
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
    while ((item = layout->takeAt(0)) != nullptr) {      //remove the first element until none remain
        if (QWidget *widget = item->widget()) {         //if it is a simple widget, remove it
            input_list.removeOne(widget);
            item->widget()->deleteLater();
        }
        else if (item->layout())    //if it is a nested layout, clear and remove it
        {
            clearLayout(item->layout());
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
    if (range <= 0.0) //prevent negative or 0 value
        return 1.0;

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
    if (!m_chartView || !m_chartView->viewport()){  //check if the graph exist
        return QObject::eventFilter(obj, event);
    }
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

        if (m_cursorLine) {
            QPointF p1 = m_chart->mapToPosition(QPointF(x, yMin), m_series);
            QPointF p2 = m_chart->mapToPosition(QPointF(x, yMax), m_series);
            m_cursorLine->setLine(QLineF(p1, p2));
        }
        m_cursorLine->setLine(QLineF(p1, p2));
    }

    //handle mouse click
    else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            QPointF value = m_chart->mapToValue(mouseEvent->pos(), m_series);
            if (m_zoomMode) {
                if (m_zoomClickCount == 0) {
                    // Premier clic
                    m_firstZoomPoint = value;
                    m_zoomClickCount = 1;

                    // Optionnel : afficher un message temporaire
                    ui->point_value_output->setText(
                        QString("Zoom: Premier point sélectionné (x=%1, y=%2) - Cliquez pour le second point")
                            .arg(m_firstZoomPoint.x(), 0, 'f', 2)
                            .arg(m_firstZoomPoint.y(), 0, 'f', 2)
                        );
                }
                else if (m_zoomClickCount == 1) {
                    // Deuxième clic
                    QPointF secondZoomPoint = value;
                    m_zoomClickCount = 0;
                    m_zoomMode = false;

                    // Appliquer le zoom sur la région sélectionnée
                    applyZoomRegion(m_firstZoomPoint, secondZoomPoint);

                    // Désactiver visuellement le bouton
                    ui->zoom_button->setChecked(false);
                    m_chartView->viewport()->setCursor(Qt::ArrowCursor);
                }
            }
            else if (ui->startendbutton->isChecked()){       //if integration button checked
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

void MainWindow::applyZoomRegion(const QPointF& p1, const QPointF& p2)
{
    double xmin = qMin(p1.x(), p2.x());
    double xmax = qMax(p1.x(), p2.x());
    double ymin = qMin(p1.y(), p2.y());
    double ymax = qMax(p1.y(), p2.y());

    double xMargin = (xmax - xmin) * UIConstants::ZOOM_MARGIN_FACTOR;
    double yMargin = (ymax - ymin) * UIConstants::ZOOM_MARGIN_FACTOR;

    xmin -= xMargin;
    xmax += xMargin;
    ymin -= yMargin;
    ymax += yMargin;

    ui->input_min->setValue(xmin);
    ui->input_max->setValue(xmax);

    ui->manual_y_limit->setChecked(true);
    ui->input_ymin->setValue(ymin);
    ui->input_ymax->setValue(ymax);

    refreshSeries();
    refreshAxes();
}

void MainWindow::initGraph(){
    //x and y series
    m_series = new QLineSeries(this);
    m_chart  = new QChart();

    //y=0 line should be first to be under all the other series
    m_zeroSeries = new QLineSeries(this);
    m_zeroSeries->setColor(Qt::black);
    m_chart->addSeries(m_zeroSeries);

    // area between curve and y=0
    m_integrationBaseSeries = new QLineSeries(this);
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

    for (int i = 0; i < UIConstants::MAX_DEGREE; i++) {
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
            box->blockSignals(true);
            box->setValue(v / 10.0);
            box->blockSignals(false);
            refreshGraph();
        });
        connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged),this, [=](double d) {
            slider->blockSignals(true);
            slider->setValue(d * 10);
            slider->blockSignals(false);
            refreshGraph();
        });

        //x value
        QLabel *test = new QLabel();
        if (i==UIConstants::MAX_DEGREE-1){test->setText("x1");}
        else if (i==UIConstants::MAX_DEGREE-2){test->setText("x");}
        else {test->setText("x^"+QString::number(UIConstants::MAX_DEGREE-i-1));}

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

    if (UIConstants::NUMBER_POINTS <= 1) {
        qWarning() << "UIConstants::NUMBER_POINTS must be at least 2";
        return;
    }

    double step = (xmax - xmin) / (UIConstants::NUMBER_POINTS - 1);

    QList<QPointF> newPoints, newPointsFirst, newPointsSecond, newPointsTangent;
    newPoints.reserve(UIConstants::NUMBER_POINTS);

    //apply the function to every x point
    for (int i = 0; i < UIConstants::NUMBER_POINTS; i++) {
        double x = xmin + i * step;
        newPoints.append(QPointF(x, m_calculator.evaluate(x)));
        newPointsFirst.append(QPointF(x, m_calculator.evaluate(x, 1)));    //first derivative should always be calculated for the extremas but not alwasy displayed

        //if thz second derivative should be displayed
        if (ui->box_derivate_second->isChecked()) {
            newPointsSecond.append(QPointF(x, m_calculator.evaluate(x, 2)));
        }
        //if the tangent should be displayed
        if (ui->box_tangent->isChecked()) {
            double a = ui->selected_x->value();
            newPointsTangent.append(QPointF(x, m_calculator.evaluateTangent(x, a)));
        }
    }

    if (ui->box_derivate_first->isChecked()) {      //first derivative should always be calculated for the extremas but not alwasy displayed
        m_seriesfirst->replace(newPointsFirst);
    }else{
        m_seriesfirst->replace({});
    }
    m_series->replace(newPoints);
    m_seriessecond->replace(newPointsSecond);
    m_seriestangent->replace(newPointsTangent);

    //part to determine roots
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setHeaderData(0, Qt::Horizontal, "roots");

    QList<double> roots_list = m_calculator.findAllRoots(m_calculator.approximateX0(newPoints),0);
    model->setRowCount(roots_list.size());

    for (int i=0; i<roots_list.size();i++){
        QStandardItem *item = new QStandardItem(QString::number(roots_list[i], 'f', 6));
        model->setItem(i, 0, item);
    }
    QStandardItemModel *oldModel = qobject_cast<QStandardItemModel*>(ui->roots_table->model());
    ui->roots_table->setModel(model);
    if (oldModel) {
        oldModel->deleteLater();
    }


    //part to determine extrema
    QStandardItemModel *model_extr = new QStandardItemModel(this);
    model_extr->setColumnCount(1);
    model_extr->setHeaderData(0, Qt::Horizontal, "extremas");

    QList<double> extrema_list = m_calculator.findAllRoots(m_calculator.approximateX0(newPointsFirst),1);
    model_extr->setRowCount(extrema_list.size());

    for (int i=0; i<extrema_list.size();i++){
        QStandardItem *item = new QStandardItem(QString::number(extrema_list[i], 'f', 6));
        model_extr->setItem(i, 0, item);
    }
    QStandardItemModel *oldModelExtr = qobject_cast<QStandardItemModel*>(ui->extrema_table->model());
    ui->extrema_table->setModel(model_extr);
    if (oldModelExtr) {
        oldModelExtr->deleteLater();
    }
}

void MainWindow::refreshAxes(){
    double xmin = ui->input_min->value();
    double xmax = ui->input_max->value();

    QVector<QPointF> newPoints = m_series->points();

    if (newPoints.isEmpty()){
        return;
    }

    double ymin;
    double ymax;
    if (ui->manual_y_limit->isChecked()){
        ymin=ui->input_ymin->value();
        ymax=ui->input_ymax->value();
    }else {
        //get y min and max values
        auto [minIt, maxIt] = std::minmax_element(
            newPoints.begin(), newPoints.end(),
            [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); }
            );


        //apply the axis limit
        //if the degree is 0, adjust y min and max for visibility
        ymin=minIt->y();
        ymax=maxIt->y();

        if (ymin==ymax){
            ymin=ymin-5;
            ymax=ymax+5;
        }else {
            ymin=ymin*UIConstants::DEFAULT_Y_MARGIN_FACTOR;
            ymax=ymax*UIConstants::DEFAULT_Y_MARGIN_FACTOR;
        }

        ui->input_ymin->blockSignals(true);
        ui->input_ymax->blockSignals(true);
        ui->input_ymin->setValue(ymin);
        ui->input_ymax->setValue(ymax);
        ui->input_ymin->blockSignals(false);
        ui->input_ymax->blockSignals(false);
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
    ui->integration_equation_output->setText(m_calculator.polynomialToString(m_calculator.getIntegrationCoefficients(),-1));
    ui->integrationvalue_output->setValue(m_calculator.evaluateIntegration(ui->start_integration_input->value(),ui->end_integration_input->value()));
}

void MainWindow::move_select_point(){
    double x = ui->selected_x->value();
    double y = m_calculator.evaluate(x);

    m_selectedPoint->clear();
    m_selectedPoint->append(x,y);

    ui->selected_y->setValue(y);
    ui->selected_tangent->setValue(m_calculator.evaluate(x,1));
    ui->selected_der1->setValue(m_calculator.evaluate(x,1));
    ui->selected_der2->setValue(m_calculator.evaluate(x,2));
}

void MainWindow::updateCalculatorCoefficients(){
    int degree = ui->input_input->value();
    QList<double> coeffs;

    for (int i = 0; i < degree; i++) {
        coeffs.append(0.0);
    }

    int max = ui->input_layout->count();
    int startIdx = max - degree;
    for (int i = startIdx; i < max; i++){
        if (auto *container = qobject_cast<QWidget*>(ui->input_layout->itemAt(i)->widget())) {
            if (auto *spinBox = container->findChild<QDoubleSpinBox*>()) {
                coeffs[i - startIdx] = spinBox->value();
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

    if (start == end) return; // prevent division 0

    QVector<QPointF> top;
    QVector<QPointF> bottom;

    int N = UIConstants::NUMBER_POINTS;
    double step = (end - start) / (N - 1);

    for (int i = 0; i < N; i++) {
        double x = start + i * step;
        double y = m_calculator.evaluate(x);


        if (std::isfinite(y)) {
            top.append(QPointF(x,y));
            bottom.append(QPointF(x,0));
        }
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
        ui->input_max->blockSignals(true);
        ui->input_max->setValue(value+1);
        ui->input_max->blockSignals(false);
    }
    refreshSeries();
    refreshAxes();
}

void MainWindow::on_input_max_valueChanged(double value) {
    if (value <= ui->input_min->value()) {
        ui->input_min->blockSignals(true);
        ui->input_min->setValue(value-1);
        ui->input_min->blockSignals(false);
    }
    refreshSeries();
    refreshAxes();
}

void MainWindow::on_manual_y_limit_stateChanged(int arg1)
{
    bool enabled = (arg1 == Qt::Checked);
    ui->grid_Y_limit->setEnabled(enabled);

    if (enabled) {
        double ymin = ui->input_ymin->value();
        double ymax = ui->input_ymax->value();
        if (ymin >= ymax) {
            ui->input_ymax->setValue(ymin + 1.0);
        }
    }
    refreshAxes();
}

void MainWindow::on_input_ymin_valueChanged(double value)
{
    ui->input_ymax->blockSignals(true);
    if (value >= ui->input_ymax->value()-.0001) {
        ui->input_ymax->setValue(value+.1);
    }
    ui->input_ymax->blockSignals(false);
    refreshSeries();
    refreshAxes();
}

void MainWindow::on_input_ymax_valueChanged(double value)
{
    ui->input_ymin->blockSignals(true);
    if (value <= ui->input_ymin->value()+.0001) {
        ui->input_ymin->setValue(value-.1);
    }
    ui->input_ymin->blockSignals(false);
    refreshSeries();
    refreshAxes();
}

void MainWindow::on_zoom_button_clicked()
{
    if (ui->zoom_button->isChecked()) {
        // Activer le mode zoom
        m_zoomMode = true;
        m_zoomClickCount = 0;

        // Désactiver les autres modes qui pourraient interférer
        if (ui->startendbutton->isChecked()) {
            ui->startendbutton->setChecked(false);
            m_integration_point_counter = 0;
        }

        // Optionnel : changer le curseur pour indiquer le mode zoom
        m_chartView->viewport()->setCursor(Qt::CrossCursor);
    }
    else {
        // Désactiver le mode zoom
        m_zoomMode = false;
        m_zoomClickCount = 0;

        // Restaurer le curseur
        m_chartView->viewport()->setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::on_pushButton_clicked()
{
    ui->manual_y_limit->setChecked(false);
    ui->input_min->setValue(UIConstants::DEFAULT_X_MIN);
    ui->input_max->setValue(UIConstants::DEFAULT_X_MAX);
    refreshSeries();
    refreshAxes();
}

