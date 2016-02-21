package:
	mvn package

all:
	mvn deploy
	cd ../../repo/; git add de
	cd ../../repo/; git commit -m "Updating JRift jar"
	
clean:
	mvn clean
	
run:
	mvn exec:exec
