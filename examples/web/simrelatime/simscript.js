function simScript() {
    // Retrieve the JSON data from the DOM element
    const jsonData = JSON.parse(document.getElementById('relatime').textContent);

    const drivers = jsonData.drivers;

    // save off player info
    const playerName = drivers[0].driver;
    const playerPos = drivers[0].pos;
    const playerLap = drivers[0].lap;
    const playerSpeed = drivers[0].speed;

    /** drivers sorted without the static player **/
    const sortedDrivers = drivers.slice(1).sort((a, b) => a.lap - b.lap || a.pos - b.pos);
    playerIdx = sortedDrivers.findIndex(driver => driver.driver === playerName);

    /** drivers behind array **/
    behindDrivers = sortedDrivers.slice(Math.max((playerIdx - 3),0), playerIdx);
    neededDrivers = 3 - behindDrivers.length;
    behindDrivers.unshift(...sortedDrivers.slice(sortedDrivers.length - neededDrivers));

    /** drivers ahead array **/
    aheadDrivers = sortedDrivers.slice(Math.min( (playerIdx + 1), (sortedDrivers.length) ), Math.min( (playerIdx + 4), (sortedDrivers.length) ));
    neededDrivers = 3 - aheadDrivers.length;
    aheadDrivers.push(...sortedDrivers.slice(0, neededDrivers + 1));

    behindDrivers.length = 3;
    aheadDrivers.length = 3;

    const tbody = document.getElementById('driverTable').getElementsByTagName('tbody')[0];

    // Clear any existing rows in the table body
    tbody.innerHTML = '';

    /** drivers ahead **/
    aheadDrivers.forEach(driver => {

        /** calculate gap **/
        let distance = driver.pos - playerPos;

        if (driver.pos < playerPos) {
            distance = driver.pos + (jsonData.track.ticks - playerPos);
        }
        if (driver.pos < playerPos && driver.lap > playerLap) {
            distance = driver.pos + (jsonData.track.ticks - playerPos);
        }

        let truedistance = distance * (jsonData.track.distance / jsonData.track.ticks);

        let gap = 0;
        if(playerSpeed >= 1)
        {
            gap = truedistance / playerSpeed;
        }

        /** add row **/
        const row = document.createElement('tr');

        const driverCell = document.createElement('td');
        driverCell.textContent = driver.driver;
        row.appendChild(driverCell);

        /** for debugging
        const posCell = document.createElement('td');
        posCell.textContent = driver.pos;
        row.appendChild(posCell);
        **/

        const gapCell = document.createElement('td');
        gapCell.textContent = gap.toFixed(2);
        row.appendChild(gapCell);

        tbody.appendChild(row);
    });
    
    /** add row for player **/
    const row = document.createElement('tr');
       
    const driverCell = document.createElement('td');
    driverCell.textContent = playerName;
    row.appendChild(driverCell);
        
    /** for debugging
    const posCell = document.createElement('td');
    posCell.textContent = playerPos;
    row.appendChild(posCell);
    **/
        
    const gapCell = document.createElement('td');
    gapCell.textContent = "0.00";
    row.appendChild(gapCell);
   
    tbody.appendChild(row);

    /** drivers behind **/
    behindDrivers.forEach(driver => {
        /** calculate gap **/
        let distance = playerPos - driver.pos;
        if (driver.pos > playerPos) {
            distance = playerPos + (jsonData.track.ticks - driver.pos);
        }
        let truedistance = distance * (jsonData.track.distance / jsonData.track.ticks);

        let gap = 0;
        if(playerSpeed >= 1)
        {
            gap = truedistance / playerSpeed;
        }

        /** add row **/
        const row = document.createElement('tr');

        const driverCell = document.createElement('td');
        driverCell.textContent = driver.driver;
        row.appendChild(driverCell);

        /** for debugging
        const posCell = document.createElement('td');
        posCell.textContent = driver.pos;
        row.appendChild(posCell);
        **/

        const gapCell = document.createElement('td');
        gapCell.textContent = gap.toFixed(2);
        row.appendChild(gapCell);

        tbody.appendChild(row);
    });
}
