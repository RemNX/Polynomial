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

void MainWindow::clearLayout(QLayout *layout)
{
    if (layout == nullptr)
        return;

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->widget())
            delete item->widget();

        else if (item->layout())
        {
            clearLayout(item->layout());
            delete item->layout();
        }

        delete item;
    }
}


void MainWindow::init_graph(){
    m_series = new QLineSeries();
    m_chart  = new QChart();
    m_chart->addSeries(m_series);

    m_axisX = new QValueAxis();
    m_axisX->setTitleText("x");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("y");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    ui->display_graph->addWidget(m_chartView);
}


void MainWindow::refresh_graph(){
    //default values
    double xmin=-2;
    double xmax=4;
    int npoints=100;

    double step = (xmax-xmin)/(npoints-1);

    QList<QPointF> newPoints;
    for (int i = 0; i < npoints; i++){
        double x = xmin + i * step;
        newPoints.append(QPointF(x, polynomial_function(x)));
    }
    m_series->replace(newPoints);

    //get min and max
    auto [minIt, maxIt] = std::minmax_element(
        newPoints.begin(), newPoints.end(),
        [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); }
        );

    m_axisX->setRange(xmin, xmax);
    m_axisY->setRange(minIt->y(), maxIt->y());


    QList<double> coeffs;
    for (int i = 0; i < ui->input_layout->count(); i++)
    {
        QWidget  *container = ui->input_layout->itemAt(i)->widget();
        if (!container) continue;
        QDoubleSpinBox *spinBox   = container->findChild<QDoubleSpinBox*>();
        if (spinBox)
            coeffs.append(spinBox->value());
    }

    ui->Title->setText(polynomialToString(coeffs));
}


void MainWindow::on_input_input_valueChanged(int value){
    QLayout *layout = ui->input_layout;
    clearLayout(layout);

    ui->output_degree->setText(QString::number(value));

    for (int i = 0; i < value; i++)
    {
        QWidget *container = new QWidget();
        QVBoxLayout *vLayout = new QVBoxLayout(container);
        vLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *label = new QLabel();
        label->setText(QString("x<sup>%1</sup>").arg(i));


        QDoubleSpinBox *spinBox = new QDoubleSpinBox();
        spinBox->setMinimum(-5.0);
        spinBox->setMaximum(5.0);
        spinBox->setSingleStep(0.1);
        spinBox->setDecimals(2);

        QSlider *slider = new QSlider(Qt::Horizontal);
        slider->setMinimum(-500);   // -5.0 * 100
        slider->setMaximum(500);    //  5.0 * 100
        slider->setSingleStep(10);  //  0.1 * 100

        // Sync slider → spinBox (silencieux)
        connect(slider, &QSlider::valueChanged, spinBox, [spinBox](int v) {
            spinBox->blockSignals(true);
            spinBox->setValue(v / 100.0);
            spinBox->blockSignals(false);

        });

        // Sync spinBox → slider (silencieux)
        connect(spinBox, &QDoubleSpinBox::valueChanged, slider, [slider](double v) {
            slider->blockSignals(true);
            slider->setValue(static_cast<int>(v * 100));
            slider->blockSignals(false);
        });

        // Un seul point de déclenchement pour chaque source
        connect(spinBox, &QDoubleSpinBox::valueChanged, this, [this](double) {
            refresh_graph();
        });
        connect(slider, &QSlider::valueChanged, this, [this](int) {
            refresh_graph();
        });

        vLayout->addWidget(spinBox);
        vLayout->addWidget(slider);
        layout->addWidget(container);
    }
}


double MainWindow::polynomial_function(double x){
    double res=0;

    //get spinbox values for each coefficients
    QList<double> coeffs;
    for (int i = 0; i < ui->input_layout->count(); i++){

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


//ai function
QString MainWindow::polynomialToString(const QList<double> &coeffs)
{
    if (coeffs.isEmpty()) return "";

    QString result = "f(x) = ";
    int degree = coeffs.size();
    bool firstTerm = true;

    for (int i = 0; i < degree; i++)
    {
        double coeff    = coeffs[i];
        int    exponent = degree - i;

        if (coeff == 0.0) continue;

        if (!firstTerm)
            result += (coeff > 0) ? " + " : " - ";
        else if (coeff < 0)
            result += "-";

        double absCoeff = std::abs(coeff);

        if (absCoeff != 1.0 || exponent == 0)
            result += QString::number(absCoeff, 'g', 4); // 4 chiffres significatifs

        if (exponent == 1)
            result += "x";
        else if (exponent > 1)
            result += "x^" + QString::number(exponent);

        firstTerm = false;
    }

    if (firstTerm) return "f(x) = 0";
    return result;
}
