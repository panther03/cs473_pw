all: report/report.pdf PW4.zip
.PHONY: PW4.zip

report/report.pdf: report/report.md
	cd report && pandoc report.md -o report.pdf --pdf-engine=xelatex

PW4.zip:
	cd ../ && zip -r PW4/PW4.zip PW4/programs/sweep PW4/programs/tasks