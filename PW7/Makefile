all: report/report.pdf PW7.zip
.PHONY: PW7.zip

report/report.pdf: report/report.md
	cd report && pandoc report.md -o report.pdf --pdf-engine=xelatex

PW7.zip: report/report.pdf 
	cp report/report.pdf .
	cd ../ && zip -r PW7/PW7.zip PW7/project/ PW7/report.pdf PW7/output.txt