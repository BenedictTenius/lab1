#include "window.h"
#include "utils/input.h"
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

class Form : public QDialog {
public:
    Form(QWidget* parent, const QString& title) : QDialog(parent), layout(new QFormLayout(this))
    {
        setWindowTitle(title);
    }

    template <typename T>
    T* add(const QString& label, T* widget)
    {
        layout->addRow(label, widget);
        return widget;
    }

    bool ask()
    {
        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addRow(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        if (exec() != QDialog::Accepted)
            return false;
        QString values = windowTitle() + ":";
        for (int row = 0; row + 1 < layout->rowCount(); ++row) {
            QWidget* field = layout->itemAt(row, QFormLayout::FieldRole)->widget();
            QVariant value = field->property("currentText");
            if (!value.isValid())
                value = field->inherits("QCheckBox") ? field->property("checked") : field->property("text");
            values += " " + layout->labelForField(field)->property("text").toString() + "=" + value.toString();
        }
        logAction(values.toStdString());
        return true;
    }

private:
    QFormLayout* layout;
};

static QSpinBox* intSpin(int min, int max)
{
    auto* spin = new QSpinBox;
    spin->setRange(min, max);
    return spin;
}

static QDoubleSpinBox* doubleSpin(double min, double max, double value)
{
    auto* spin = new QDoubleSpinBox;
    spin->setDecimals(3);
    spin->setRange(min, max);
    spin->setValue(value);
    return spin;
}

static QComboBox* diameters(int fixed)
{
    auto* box = new QComboBox;
    for (int diameter : DIAMETERS)
        box->addItem(QString::number(diameter));
    if (fixed) {
        box->setCurrentText(QString::number(fixed));
        box->setEnabled(false);
    }
    return box;
}

static QHBoxLayout* row(std::initializer_list<QWidget*> widgets)
{
    auto* layout = new QHBoxLayout;
    for (QWidget* widget : widgets)
        layout->addWidget(widget);
    layout->addStretch();
    return layout;
}

static void setupTable(QTableWidget* table, const QStringList& headers)
{
    table->setHorizontalHeaderLabels(headers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();
}

static void fill(QTableWidget* table, const std::vector<QStringList>& rows)
{
    table->setRowCount(int(rows.size()));
    for (int r = 0; r < int(rows.size()); ++r)
        for (int c = 0; c < rows[r].size(); ++c)
            table->setItem(r, c, new QTableWidgetItem(rows[r][c]));
}

Window::Window(Network& network) : network(network)
{
    setWindowTitle("Gas pipeline network");
    resize(950, 600);
    auto* tabs = new QTabWidget;
    tabs->addTab(pipesTab(), "Pipes");
    tabs->addTab(stationsTab(), "Stations");
    auto* central = new QWidget;
    auto* layout = new QVBoxLayout(central);
    layout->addLayout(row({button("Load", [this] { load(); }), button("Save", [this] { save(); }),
                           button("Topological sort", [this] { topologicalSort(); }),
                           button("Max flow", [this] { maxFlow(); }),
                           button("Shortest path", [this] { shortestPath(); })}));
    layout->addWidget(tabs);
    setCentralWidget(central);
    refresh();
}

QWidget* Window::pipesTab()
{
    setupTable(pipeTable, {"ID", "Name", "Length, km", "Diameter, mm", "Status", "From", "To"});
    pipeName->setPlaceholderText("Name contains");
    pipeRepair->addItems({"Any", "In repair", "Working"});
    connect(pipeName, &QLineEdit::returnPressed, this, [this] { refresh(); });
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->addLayout(row({pipeName, pipeRepair, button("Search", [this] {
        logAction("Pipe filter: name=" + pipeName->text().toStdString() + " status=" + pipeRepair->currentText().toStdString());
    })}));
    layout->addWidget(pipeTable);
    layout->addLayout(row({
        button("Add", [this] { addPipe(); }),
        button("Set in repair", [this] { for (int id : selected(pipeTable)) network.setInRepair(id, true); }),
        button("Set working", [this] { for (int id : selected(pipeTable)) network.setInRepair(id, false); }),
        button("Delete", [this] { for (int id : selected(pipeTable)) network.deletePipe(id); }),
        button("Connect stations", [this] { connectStations(); }),
        button("Disconnect", [this] { for (int id : selected(pipeTable)) network.disconnect(id); }),
    }));
    return tab;
}

QWidget* Window::stationsTab()
{
    setupTable(stationTable, {"ID", "Name", "Workshops", "Working", "Unused, %", "Class"});
    stationName->setPlaceholderText("Name contains");
    unusedFrom->setRange(0, 100);
    unusedTo->setRange(0, 100);
    unusedTo->setValue(100);
    unusedFrom->setPrefix("Unused from ");
    unusedTo->setPrefix("to ");
    connect(stationName, &QLineEdit::returnPressed, this, [this] { refresh(); });
    auto workshops = [this](bool start) {
        QString failed;
        for (int id : selected(stationTable))
            if (!network.changeWorkshops(id, start))
                failed += " #" + QString::number(id);
        if (!failed.isEmpty())
            QMessageBox::information(this, "Workshops", "No workshop to " + QString(start ? "start" : "stop") + " at" + failed);
    };
    auto* tab = new QWidget;
    auto* layout = new QVBoxLayout(tab);
    layout->addLayout(row({stationName, unusedFrom, unusedTo, button("Search", [this] {
        logAction("Station filter: name=" + stationName->text().toStdString() + " unused="
                  + std::to_string(unusedFrom->value()) + ".." + std::to_string(unusedTo->value()));
    })}));
    layout->addWidget(stationTable);
    layout->addLayout(row({
        button("Add", [this] { addStation(); }),
        button("Start workshop", [workshops] { workshops(true); }),
        button("Stop workshop", [workshops] { workshops(false); }),
        button("Delete", [this] { for (int id : selected(stationTable)) network.deleteStation(id); }),
    }));
    return tab;
}

QPushButton* Window::button(const QString& text, std::function<void()> action)
{
    auto* result = new QPushButton(text);
    connect(result, &QPushButton::clicked, this, [this, text, action] {
        logAction("GUI: " + text.toStdString());
        try {
            action();
        } catch (const std::exception& error) {
            QMessageBox::warning(this, text, error.what());
        }
        refresh();
    });
    return result;
}

void Window::refresh()
{
    std::vector<QStringList> rows;
    for (int id : network.findPipes(pipeName->text().toStdString(), pipeRepair->currentIndex())) {
        const Pipe& pipe = network.getPipes().at(id);
        rows.push_back({QString::number(id), QString::fromStdString(pipe.name), QString::number(pipe.length),
                        QString::number(pipe.diameter), pipe.inRepair ? "In repair" : "Working",
                        pipe.isFree() ? "" : QString::number(pipe.from), pipe.isFree() ? "" : QString::number(pipe.to)});
    }
    fill(pipeTable, rows);
    rows.clear();
    for (int id : network.findStations(stationName->text().toStdString(), unusedFrom->value(), unusedTo->value())) {
        const Station& station = network.getStations().at(id);
        rows.push_back({QString::number(id), QString::fromStdString(station.name), QString::number(station.workshops),
                        QString::number(station.working), QString::number(station.unusedPercent(), 'f', 1),
                        QString::number(station.stationClass)});
    }
    fill(stationTable, rows);
}

std::set<int> Window::selected(QTableWidget* table) const
{
    std::set<int> ids;
    std::string text = "Selected ids:";
    for (const QModelIndex& index : table->selectionModel()->selectedRows()) {
        ids.insert(table->item(index.row(), 0)->text().toInt());
        text += " " + table->item(index.row(), 0)->text().toStdString();
    }
    if (ids.empty())
        throw std::runtime_error("Select rows in the table (Ctrl+A - all found)");
    logAction(text);
    return ids;
}

QComboBox* Window::stationBox() const
{
    auto* box = new QComboBox;
    for (const auto& [id, station] : network.getStations())
        box->addItem("#" + QString::number(id) + " " + QString::fromStdString(station.name), id);
    return box;
}

bool Window::askRoute(const QString& title, int& from, int& to)
{
    if (network.getStations().size() < 2)
        throw std::runtime_error("Need at least 2 stations");
    Form form(this, title);
    auto* fromBox = form.add("From station", stationBox());
    auto* toBox = form.add("To station", stationBox());
    toBox->setCurrentIndex(1);
    if (!form.ask())
        return false;
    from = fromBox->currentData().toInt();
    to = toBox->currentData().toInt();
    network.checkRoute(from, to);
    return true;
}

void Window::addPipe(int diameter, int from, int to)
{
    Form form(this, "Add pipe");
    auto* name = form.add("Name (km mark)", new QLineEdit);
    auto* length = form.add("Length, km", doubleSpin(0.001, 100000, 10));
    auto* diameterBox = form.add("Diameter, mm", diameters(diameter));
    auto* inRepair = form.add("In repair", new QCheckBox);
    if (!form.ask())
        return;
    if (name->text().trimmed().isEmpty())
        throw std::runtime_error("Name can't be empty");
    Pipe pipe;
    pipe.name = name->text().toStdString();
    pipe.length = length->value();
    pipe.diameter = diameterBox->currentText().toInt();
    pipe.inRepair = inRepair->isChecked();
    int id = network.addPipe(pipe);
    if (from)
        network.connect(id, from, to);
}

void Window::addStation()
{
    Form form(this, "Add station");
    auto* name = form.add("Name", new QLineEdit);
    auto* workshops = form.add("Workshops", intSpin(1, 1000));
    auto* working = form.add("Working workshops", intSpin(0, 1));
    auto* stationClass = form.add("Class", intSpin(1, 5));
    connect(workshops, &QSpinBox::valueChanged, working, &QSpinBox::setMaximum);
    if (!form.ask())
        return;
    if (name->text().trimmed().isEmpty())
        throw std::runtime_error("Name can't be empty");
    Station station;
    station.name = name->text().toStdString();
    station.workshops = workshops->value();
    station.working = working->value();
    station.stationClass = stationClass->value();
    network.addStation(station);
}

void Window::connectStations()
{
    if (network.getStations().size() < 2)
        throw std::runtime_error("Need at least 2 stations");
    Form form(this, "Connect stations");
    auto* fromBox = form.add("Input station", stationBox());
    auto* toBox = form.add("Output station", stationBox());
    auto* diameterBox = form.add("Diameter, mm", diameters(0));
    toBox->setCurrentIndex(1);
    if (!form.ask())
        return;
    int from = fromBox->currentData().toInt(), to = toBox->currentData().toInt();
    int diameter = diameterBox->currentText().toInt();
    network.checkRoute(from, to);
    if (int id = network.findFreePipe(diameter)) {
        network.connect(id, from, to);
        return;
    }
    QMessageBox::information(this, "Connect stations", "No free pipe with this diameter, create a new one");
    addPipe(diameter, from, to);
}

void Window::load()
{
    QString path = QFileDialog::getOpenFileName(this, "Load", ".", "JSON (*.json);;All files (*)");
    if (!path.isEmpty())
        network.load(path.toStdString());
}

void Window::save()
{
    QString path = QFileDialog::getSaveFileName(this, "Save", "network.json", "JSON (*.json)");
    if (!path.isEmpty())
        network.save(path.toStdString());
}

void Window::topologicalSort()
{
    std::vector<int> order;
    if (!network.graph().topologicalSort(order))
        throw std::runtime_error("Network has a cycle, topological sort is impossible");
    QString text = "Topological order:";
    for (int station : order)
        text += " " + QString::number(station);
    QMessageBox::information(this, "Topological sort", text);
}

void Window::maxFlow()
{
    int from, to;
    if (askRoute("Max flow", from, to))
        QMessageBox::information(this, "Max flow", QString("Max flow from %1 to %2: %3")
                                                       .arg(from).arg(to).arg(network.graph().maxFlow(from, to)));
}

void Window::shortestPath()
{
    int from, to;
    if (!askRoute("Shortest path", from, to))
        return;
    double length = 0;
    std::vector<int> path = network.graph().shortestPath(from, to, length);
    if (path.empty())
        throw std::runtime_error("No path from " + std::to_string(from) + " to " + std::to_string(to));
    QString text = "Shortest path (" + QString::number(length) + " km):";
    for (int station : path)
        text += " " + QString::number(station);
    QMessageBox::information(this, "Shortest path", text);
}
