/*
    Project Kawa Client | CAPTIVE PORTAL JS

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

Vue.component('spinner', {
    template: '#spinner-template',
    props: ['spinnerText']
});

Vue.component('missing', {
    template: '#svg-missing',
    props: ['tooltip']
})

Vue.component('warning', {
    template: '#svg-warning',
    props: ['tooltip']
})

new Vue({
    el: "#app",
    
    data: {
        debug: false,
        vueLoaded: false,
        titleText: 'Project Kawa',

        overlay: {
            show: true,
            title: null,
            text: null
        },
        
        show: {
            warning: {
                ssid: false,
                sensor: false
            },
            missing: {
                ssid: false,
                password: false,
                address: false,
                port: false,
                sensor: false,
                interval: false
            }
        },

        settings: {
            ssid: "",
            password: "",
            address: "",
            port: "",
            sensor: "",
            interval: "",
        },

        device: {},

        available: {
            ssid: [],
            sensor: [],
            interval: []
        },

        selectable: {
            ssid: [],
            sensor: [],
            interval: []
        }
    },

    mounted() {
        this.vueLoaded = true;
        if (this.debug) {
            this.titleText = 'DEBUG MODE';
            this.overlay.show = false;
        }
        else {
            setTimeout(this.request_get.bind(this), 100);
        }
    },

    methods: {
        timeout_reset_overlay() {
            this.reset_overlay();
        },

        timeout_reset_missing() {
            for (var prop in this.show.missing) {
                this.show.missing[prop] = false;
            }
        },

        set_overlay_text(title, text) {
            this.overlay.title = title;
            this.overlay.text = text;
        },

        reset_overlay() {
            this.overlay.show = false;
            this.overlay.title = null;
            this.overlay.text = null;
        },

        onclick_reset_overlay() {
            if (this.overlay.title != null || this.overlay.text != null) {
                this.reset_overlay();
            }
        },

        commit() {
            if (this.settings.ssid == "" || this.settings.password == "" || this.settings.address == "" || this.settings.port == "" || this.settings.sensor == "" || this.settings.interval == "") {
                if (this.settings.ssid == "") {
                    this.show.missing.ssid = true;
                }
                if (this.settings.password == "") {
                    this.show.missing.password = true;
                }
                if (this.settings.address == "") {
                    this.show.missing.address = true;
                }
                if (this.settings.port == "") {
                    this.show.missing.port = true;
                }
                if (this.settings.sensor == "") {
                    this.show.missing.sensor = true;
                }
                if (this.settings.interval == "") {
                    this.show.missing.interval = true;
                }
                setTimeout(this.timeout_reset_missing.bind(this), 6000);
            }
            else {
                this.request_post();
            }
        },

        discard() {
            this.request_get();
        },

        restart() {
            this.request_restart();
        },

        reset() {
            this.request_delete();
        },

        request_get() {
            const vm = this;
            this.overlay.show = true;
            
            axios
                .get('/get')
                .then(response => {
                    vm.settings = response.data.settings;
                    vm.device = response.data.device;
                    vm.available = response.data.avail;

                    if (vm.settings.port == 0) {
                        vm.settings.port = "";
                    }

                    if (vm.settings.ssid != "" && !vm.available.ssid.includes(vm.settings.ssid)) {
                        vm.selectable.ssid = [vm.settings.ssid].concat(vm.available.ssid);
                    }
                    else {
                        vm.selectable.ssid = vm.available.ssid;
                    }
                    
                    if (vm.settings.sensor != "" && !vm.available.sensor.includes(vm.settings.sensor)) {
                        vm.selectable.sensor = [vm.settings.sensor].concat(vm.available.sensor);
                    }
                    else {
                        vm.selectable.sensor = vm.available.sensor;
                    }
                    
                    if (vm.settings.interval != "" && !vm.available.interval.includes(vm.settings.interval)) {
                        vm.selectable.sensor = [vm.settings.interval].concat(vm.available.interval);
                    }
                    else {
                        vm.selectable.interval = vm.available.interval;
                    }

                    setTimeout(vm.timeout_reset_overlay.bind(vm), 500);
                })
                .catch(e => {
                    console.error(e);
                    vm.set_overlay_text("Failed", e)
                    setTimeout(vm.timeout_reset_overlay.bind(vm), 5000);
                })
        },

        request_post() {
            const vm = this;
            this.overlay.show = true;

            axios
                .post('/post', {
                    body: vm.settings
                })
                .then(response => {
                    if (response.data == "OK") {
                        vm.set_overlay_text("Saved", "You may restart the controller now.");
                    }
                    else {
                        vm.set_overlay_text("Failed", "Internal server error.");
                    }
                    setTimeout(vm.timeout_reset_overlay.bind(vm), 5000);
                })
                .catch(e => {
                    console.error(e);
                    vm.set_overlay_text("Failed", e)
                    setTimeout(vm.timeout_reset_overlay.bind(vm), 5000);
                })
        },

        request_restart() {
            const vm = this;
            this.overlay.show = true;
            
            axios
                .get('/restart')
                .then(response => {
                    if (response.data == "OK") {
                        vm.set_overlay_text("Done", "You may close this site.");
                    }
                    else {
                        vm.set_overlay_text("Failed", "Internal server error.");
                    }
                    setTimeout(vm.timeout_reset_overlay.bind(vm), 5000);
                })
                .catch(e => {
                    console.error(e);
                    vm.set_overlay_text("Failed", e)
                    setTimeout(vm.timeout_reset_overlay.bind(vm), 5000);
                })
        },

        request_delete() {
            const vm = this;
            this.overlay.show = true;

            axios
                .get('/reset')
                .then(response => {
                    if (response.data == "OK") {
                        vm.set_overlay_text("Done", "All data cleared.");
                    }
                    else {
                        vm.set_overlay_text("Failed", "Internal server error.");
                    }
                    setTimeout(vm.timeout_reset_overlay.bind(vm), 5000);
                    setTimeout(vm.request_get.bind(vm), 3050);
                })
                .catch(e => {
                    console.error(e);
                    vm.set_overlay_text("Failed", e)
                    setTimeout(vm.timeout_reset_overlay.bind(vm), 5000);
                })
        },

        set_warning_ssid() {
            if (this.settings.ssid == "" || this.available.ssid.includes(this.settings.ssid)) {
                this.show.warning.ssid = false;
            }
            else {
                this.show.warning.ssid = true;
            }
        },

        set_warning_sensor() {
            if (this.settings.sensor == "" || this.available.sensor.includes(this.settings.sensor)) {
                this.show.warning.sensor = false;
            }
            else {
                this.show.warning.sensor = true;
            }
        },

        set_warning_interval() {
            if (this.settings.interval == "" || this.available.interval.includes(this.settings.interval)) {
                this.show.warning.interval = false;
            }
            else {
                this.show.warning.interval = true;
            }
        }
    },

    computed: {

    },

    watch: {
        settings: {
            handler() {
                if (this.settings.ssid != "" && this.show.warning.ssid) {
                    this.show.warning.ssid = false;
                }
                if (this.settings.sensor != "" && this.show.warning.sensor) {
                    this.show.warning.sensor = false;
                }
                if (this.settings.interval != "" && this.show.warning.interval) {
                    this.show.warning.interval = false;
                }

                this.set_warning_ssid();
                this.set_warning_sensor();
                this.set_warning_interval();
            },
            deep: true
        },

        selectable: {
            handler() {
                this.set_warning_ssid();
                this.set_warning_sensor();
                this.set_warning_interval();
            },
            deep: true
        }
    },
});